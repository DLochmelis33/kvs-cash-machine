#include "ShardBuilder.h"
#include "Storage.h"
#include "doctest.h"

#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

using namespace kvs::shard;
using namespace kvs::storage;

namespace test_kvs::shard {

const std::string testDirectoryPath = "../.test-data/test-shard/";

void setUpTestDirectory() {
  std::filesystem::create_directories(testDirectoryPath);
  Shard::storageDirectoryPath = testDirectoryPath;
}

void clearTestDirectory() { std::filesystem::remove_all(testDirectoryPath); }

std::vector<std::pair<Value, Ptr>> generateValuesAndPtrs(size_t size,
                                                         char seedChar) {
  std::srand(seedChar);
  std::vector<std::pair<Value, Ptr>> elements;
  elements.reserve(size);
  for (size_t i = 0; i < size; ++i) {
    ByteArray byteArray{VALUE_SIZE};
    std::string valueBytes(VALUE_SIZE, seedChar + i);
    std::memcpy(byteArray.get(), &valueBytes[0], VALUE_SIZE);
    elements.emplace_back(Value{byteArray}, Ptr{i, std::rand() % 2 == 0});
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
      storage.append(value.getBytes());
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

  SUBCASE("test black-box methods") {

    SUBCASE("test readValue") {}

    SUBCASE("test writeValue") {}

    SUBCASE("test removeValue") {}
  }

  SUBCASE("test complex") {}

  clearTestDirectory();
}

} // namespace test_kvs::shard
