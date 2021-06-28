#include "ShardBuilder.h"
#include "Storage.h"
#include "doctest.h"

#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <random>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace kvs::shard;
using namespace kvs::storage;

namespace std {

// required for std::unordered_map<Key>
template <>
struct hash<kvs::utils::Key> {
  std::size_t operator()(const kvs::utils::Key& key) const noexcept {
    return kvs::utils::hashKey(key, 0);
  }
};

} // namespace std

namespace test_kvs::shard {

const std::string testDirectoryPath = "../.test-data/test-shard/";

void setUpTestDirectory() {
  std::filesystem::create_directories(testDirectoryPath);
  Shard::storageDirectoryPath = testDirectoryPath;
}

void clearTestDirectory() { std::filesystem::remove_all(testDirectoryPath); }

Key generateKey(size_t value) {
  ByteArray byteArray{KEY_SIZE};
  std::memcpy(byteArray.get(), reinterpret_cast<char*>(&value), sizeof(size_t));
  return Key{byteArray};
}

Value generateValue(char seedChar) {
  ByteArray byteArray{VALUE_SIZE};
  std::string valueBytes(VALUE_SIZE, seedChar);
  std::memcpy(byteArray.get(), &valueBytes[0], VALUE_SIZE);
  return Value{byteArray};
}

std::vector<std::pair<Value, Ptr>> generateValuesAndPtrs(size_t size,
                                                         char seedChar) {
  std::srand(seedChar);
  std::vector<std::pair<Value, Ptr>> elements;
  elements.reserve(size);
  for (size_t i = 0; i < size; ++i) {
    elements.emplace_back(generateValue(seedChar + i),
                          Ptr{i * VALUE_SIZE, std::rand() % 2 == 0});
  }
  return elements;
}

TEST_CASE("test Shard") {
  setUpTestDirectory();
  shard_index_t shardIndex = 65;
  Shard shard = ShardBuilder::createShard(shardIndex);
  std::string valuesFilePath = Shard::getValuesFilePath(shardIndex);
  std::string hashTableFilePath =
      Shard::getStorageHashTableFilePath(shardIndex);

  SUBCASE("test direct methods") {
    size_t size = 20;
    std::vector<std::pair<Value, Ptr>> elements =
        generateValuesAndPtrs(size, 'a');

    values_cnt_t aliveValuesCnt = 0;
    values_cnt_t valuesCnt = 0;

    Storage storage(valuesFilePath);
    for (const auto& element : elements) {
      auto [value, ptr] = element;
      size_t offset = storage.append(value.getBytes());
      REQUIRE(offset == ptr.getOffset());

      if (ptr.isValuePresent()) {
        shard.incrementAliveValuesCnt();
        ++aliveValuesCnt;
      }
      ++valuesCnt;
      if (valuesCnt * MAX_OUTDATED_RECORDS_LOAD_FACTOR > aliveValuesCnt) {
        CHECK(shard.isRebuildRequired(shardIndex));
      } else {
        CHECK_FALSE(shard.isRebuildRequired(shardIndex));
      }
    }
    storage.close();

    SUBCASE("test readValueDirectly") {
      for (const auto& element : elements) {
        auto [value, ptr] = element;
        Value readValue = shard.readValueDirectly(shardIndex, ptr);
        CHECK(readValue == value);
      }
    }

    SUBCASE("test writeValueDirectly") {
      std::vector<std::pair<Value, Ptr>> elementsToWrite =
          generateValuesAndPtrs(size, 'A');
      for (size_t i = 0; i < size; ++i) {
        auto [value, ptr] = elementsToWrite[i];
        auto [oldValue, oldPtr] = elements[i];
        shard.writeValueDirectly(shardIndex, ptr, value);
        if (oldPtr.isValuePresent() && !ptr.isValuePresent()) {
          shard.decrementAliveValuesCnt();
          --aliveValuesCnt;
        }
        if (!oldPtr.isValuePresent() && ptr.isValuePresent()) {
          shard.incrementAliveValuesCnt();
          ++aliveValuesCnt;
        }
        if (valuesCnt * MAX_OUTDATED_RECORDS_LOAD_FACTOR > aliveValuesCnt) {
          CHECK(shard.isRebuildRequired(shardIndex));
        } else {
          CHECK_FALSE(shard.isRebuildRequired(shardIndex));
        }
      }

      Storage storage(valuesFilePath);
      for (const auto& elementToWrite : elementsToWrite) {
        auto [value, ptr] = elementToWrite;
        Value readValue{storage.read(ptr.getOffset(), VALUE_SIZE)};
        CHECK(readValue == value);
      }
      storage.close();
    }
  }

  SUBCASE("test black-box methods: fill with writeValue") {
    values_cnt_t valuesCnt = 12;

    std::vector<std::pair<Entry, Value>> elements;
    for (values_cnt_t i = 0; i < valuesCnt; ++i) {
      Key key = generateKey(i);
      Value value = generateValue(i);
      Ptr ptr{i * VALUE_SIZE, true};
      Entry entry{key, ptr};
      elements.emplace_back(entry, value);

      Entry writeEntry = shard.writeValue(shardIndex, key, value);
      REQUIRE(writeEntry.key == key);
      REQUIRE(writeEntry.ptr == ptr);
    }
    REQUIRE_FALSE(shard.isRebuildRequired(shardIndex));

    SUBCASE("& test readValue") {
      for (const auto& element : elements) {
        auto [entry, value] = element;
        auto [readEntry, readValue] = shard.readValue(shardIndex, entry.key);
        REQUIRE(readValue.has_value());
        CHECK(readValue.value() == value);
        CHECK(readEntry.key == entry.key);
        CHECK(readEntry.ptr == entry.ptr);
      }
      Key nonExistentKey = generateKey(valuesCnt + 2);
      auto [readEntry, readValue] = shard.readValue(shardIndex, nonExistentKey);
      CHECK_FALSE(readValue.has_value());
      CHECK(readEntry.key == nonExistentKey);
      CHECK(readEntry.ptr.getType() == PtrType::EMPTY_PTR);
    }

    SUBCASE("& test rewrite with writeValue and readValue") {
      for (values_cnt_t i = 0; i < valuesCnt; ++i) {
        if (i % 2 == 0) {
          elements[i].second = generateValue('A' + i);
        }
        auto [entry, value] = elements[i];
        Entry writeEntry = shard.writeValue(shardIndex, entry.key, value);
        REQUIRE(writeEntry.key == entry.key);
        REQUIRE(writeEntry.ptr == entry.ptr);
      }
      REQUIRE_FALSE(shard.isRebuildRequired(shardIndex));

      for (const auto& element : elements) {
        auto [entry, value] = element;
        auto [readEntry, readValue] = shard.readValue(shardIndex, entry.key);
        REQUIRE(readValue.has_value());
        CHECK(readValue.value() == value);
        CHECK(readEntry.key == entry.key);
        CHECK(readEntry.ptr == entry.ptr);
      }
    }

    SUBCASE("& test removeEntry with readValue") {
      for (values_cnt_t i = 0; i < valuesCnt; i += 3) {
        elements[i].first.ptr.setValuePresent(false);
        auto [entry, value] = elements[i];
        Entry removeEntry = shard.removeEntry(shardIndex, entry.key);
        REQUIRE(removeEntry.key == entry.key);
        REQUIRE(removeEntry.ptr == entry.ptr);
      }
      REQUIRE_FALSE(shard.isRebuildRequired(shardIndex));

      for (values_cnt_t i = 0; i < valuesCnt; ++i) {
        auto [entry, value] = elements[i];
        auto [readEntry, readValue] = shard.readValue(shardIndex, entry.key);
        if (i % 3 == 0) {
          REQUIRE_FALSE(readValue.has_value());
        } else {
          REQUIRE(readValue.has_value());
          CHECK(readValue.value() == value);
        }
        CHECK(readEntry.key == entry.key);
        CHECK(readEntry.ptr == entry.ptr);
      }
    }
  }

  SUBCASE("stress test blackbox methods") {
    std::unordered_map<Key, Value> kvsMap;
    std::unordered_set<Key> visitedSet;
    size_t operations = 1e4;
    size_t keyRangeUpperBound = 30;

    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<size_t> distr(0, keyRangeUpperBound);
    srand(time(NULL));

    for (size_t i = 0; i < operations; ++i) {
      Key key = generateKey(distr(gen));
      switch (rand() % 3) {
      case 0: { // write
        Value value = generateValue(rand() % std::numeric_limits<char>::max());
        auto [writeKey, writePtr] = shard.writeValue(shardIndex, key, value);
        REQUIRE(writeKey == key);
        REQUIRE(writePtr.getType() == PtrType::PRESENT);
        kvsMap[key] = value;
        visitedSet.insert(key);
        break;
      }
      case 1: { // remove
        auto [removeKey, removePtr] = shard.removeEntry(shardIndex, key);
        REQUIRE(removeKey == key);
        if (visitedSet.find(key) == visitedSet.end()) {
          REQUIRE(removePtr.getType() == PtrType::EMPTY_PTR);
        } else {
          REQUIRE(removePtr.getType() == PtrType::DELETED);
        }
        kvsMap.erase(key);
        break;
      }
      case 2: { // read
        auto [readEntry, readValue] = shard.readValue(shardIndex, key);
        auto [readKey, readPtr] = readEntry;
        const auto& iter = kvsMap.find(key);
        if (kvsMap.find(key) == kvsMap.end()) {
          REQUIRE_FALSE(readValue.has_value());
          REQUIRE(readKey == key);
          if (visitedSet.find(key) == visitedSet.end()) {
            REQUIRE(readPtr.getType() == PtrType::EMPTY_PTR);
          } else {
            REQUIRE(readPtr.getType() == PtrType::DELETED);
          }
        } else {
          REQUIRE(readValue.has_value());
          REQUIRE(readValue.value() == (*iter).second);
          REQUIRE(readKey == key);
          REQUIRE(readPtr.getType() == PtrType::PRESENT);
        }
      }
      }
    }
  }

  clearTestDirectory();
}

} // namespace test_kvs::shard
