#include "ShardBuilder.h"
#include "Storage.h"
#include "StorageHashTable.h"
#include "doctest.h"

#include <cstring>
#include <filesystem>

using namespace kvs::shard;
using namespace kvs::storage;
using kvs::cache_map::CacheMap;
using kvs::storage_hash_table::StorageHashTable;

namespace test_kvs::shard_builder {

const std::string testDirectoryPath = "../.test-data/test-shard-builder/";

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

TEST_CASE("test ShardBuilder") {
  setUpTestDirectory();

  SUBCASE("test createShard") {
    shard_index_t shardIndex = 65;
    Shard shard = ShardBuilder::createShard(shardIndex);

    std::string valuesFilePath = Shard::getValuesFilePath(shardIndex);
    REQUIRE(std::filesystem::exists(valuesFilePath));
    CHECK(std::filesystem::file_size(valuesFilePath) == 0);

    std::string hashTableFilePath =
        Shard::getStorageHashTableFilePath(shardIndex);
    REQUIRE(std::filesystem::exists(hashTableFilePath));
    CHECK(StorageHashTable{readFile(hashTableFilePath)}.getEntries().size() ==
          0);

    CHECK_FALSE(shard.isRebuildRequired(shardIndex));
  }

  SUBCASE("test rebuildShard") {
    shard_index_t shardIndex = 65;
    Shard shard = ShardBuilder::createShard(shardIndex);
    std::string valuesFilePath = Shard::getValuesFilePath(shardIndex);
    std::string hashTableFilePath =
        Shard::getStorageHashTableFilePath(shardIndex);

    CacheMap cacheMap{CACHE_MAP_SIZE};
    values_cnt_t valuesCnt = 15;
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
      std::optional<Entry> displacedEntry = cacheMap.putOrDisplace(writeEntry);
      REQUIRE_FALSE(displacedEntry.has_value());
    }
    REQUIRE_FALSE(shard.isRebuildRequired(shardIndex));

    values_cnt_t notUpdatedCacheMapRemovedEntriesCnt = 2;
    for (values_cnt_t i = 0; i < notUpdatedCacheMapRemovedEntriesCnt; ++i) {
      shard.decrementAliveValuesCnt();
      Entry newEntry = elements[i].first;
      newEntry.ptr.setValuePresent(false);
      std::optional<Entry> displacedEntry = cacheMap.putOrDisplace(newEntry);
      REQUIRE_FALSE(displacedEntry.has_value());
    }
    REQUIRE_FALSE(shard.isRebuildRequired(shardIndex));

    values_cnt_t removedValuesCnt = notUpdatedCacheMapRemovedEntriesCnt;
    while (!shard.isRebuildRequired(shardIndex)) {
      elements[removedValuesCnt].first.ptr.setValuePresent(false);
      auto [entry, value] = elements[removedValuesCnt];
      Entry removeEntry = shard.removeEntry(shardIndex, entry.key);
      REQUIRE(removeEntry.key == entry.key);
      REQUIRE(removeEntry.ptr == entry.ptr);
      std::optional<Entry> displacedEntry = cacheMap.putOrDisplace(removeEntry);
      REQUIRE_FALSE(displacedEntry.has_value());
      ++removedValuesCnt;
    }
    REQUIRE(shard.isRebuildRequired(shardIndex));

    Entry otherShardEntry{generateKey(valuesCnt), Ptr{10 * VALUE_SIZE, true}};
    std::optional<Entry> displacedEntry =
        cacheMap.putOrDisplace(otherShardEntry);
    REQUIRE_FALSE(displacedEntry.has_value());

    auto [newShard, cacheMapDiff] =
        ShardBuilder::rebuildShard(shard, shardIndex, cacheMap);

    REQUIRE(!newShard.isRebuildRequired(shardIndex));
    REQUIRE(std::filesystem::file_size(valuesFilePath) ==
            (valuesCnt - removedValuesCnt) * VALUE_SIZE);
    for (values_cnt_t presentValue = removedValuesCnt; presentValue < valuesCnt;
         ++presentValue) {
      auto [entry, value] = elements[presentValue];
      auto [readEntry, readValue] = newShard.readValue(shardIndex, entry.key);
      REQUIRE(readValue.has_value());
      CHECK(readValue.value() == value);
      CHECK(readEntry.key == entry.key);
      CHECK(readEntry.ptr.getType() == PtrType::PRESENT);
    }
    auto [readEntry, readValue] =
        newShard.readValue(shardIndex, otherShardEntry.key);
    CHECK_FALSE(readValue.has_value());
    CHECK(readEntry.key == otherShardEntry.key);
    CHECK(readEntry.ptr.getType() == PtrType::EMPTY_PTR);

    for (const auto& diffEntry : cacheMapDiff) {
      std::optional<Entry> displacedEntry = cacheMap.putOrDisplace(diffEntry);
      REQUIRE_FALSE(displacedEntry.has_value());
    }

    std::vector<Entry> newShardEntries =
        StorageHashTable{readFile(hashTableFilePath)}.getEntries();
    for (const auto& newShardEntry : newShardEntries) {
      const Key& key = newShardEntry.key;
      const Ptr& ptr = newShardEntry.ptr;
      REQUIRE(ptr.getType() == PtrType::PRESENT);
      Ptr cacheMapPtr = cacheMap.get(key);
      CHECK(cacheMapPtr == ptr);
    }

    for (values_cnt_t removedValue = 0; removedValue < removedValuesCnt;
         ++removedValue) {
      auto [entry, value] = elements[removedValue];
      auto [readEntry, readValue] = newShard.readValue(shardIndex, entry.key);
      REQUIRE_FALSE(readValue.has_value());
      CHECK(readEntry.key == entry.key);
      CHECK(readEntry.ptr.getType() == PtrType::EMPTY_PTR);
      //CHECK(cacheMap.get(entry.key).getType() == PtrType::NONEXISTENT);
      // TODO ?
    }

    Ptr otherShardEntryPtr = cacheMap.get(otherShardEntry.key);
    CHECK(otherShardEntryPtr == otherShardEntry.ptr);
  }

  clearTestDirectory();
}
} // namespace test_kvs::shard_builder
