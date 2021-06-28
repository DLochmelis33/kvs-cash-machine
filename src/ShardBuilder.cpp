#include "ShardBuilder.h"
#include "KVSException.h"
#include "Storage.h"
#include "StorageHashTable.h"

#include <cassert>
#include <filesystem>
#include <iostream>

using kvs::storage::Storage, kvs::storage_hash_table::StorageHashTable;

namespace kvs::shard {

Shard ShardBuilder::createShard(shard_index_t shardIndex) {
  try {
    std::filesystem::create_directories(
        Shard::getShardDirectoryPath(shardIndex));
  } catch (const std::exception& exc) {
    throw KVSException{KVSErrorType::FAILED_TO_CREATE_SHARD_DIRECTORY};
  }
  storage::writeFile(Shard::getValuesFilePath(shardIndex), ByteArray{0});
  storage::writeFile(
      Shard::getStorageHashTableFilePath(shardIndex),
      StorageHashTable{STORAGE_HASH_TABLE_INITIAL_SIZE}.serializeToByteArray());
  return Shard{};
}

std::pair<Shard, std::vector<Entry>>
ShardBuilder::rebuildShard(const Shard& shard, shard_index_t shardIndex,
                           const kvs::cache_map::CacheMap& cacheMap) {
  assert(shard.isRebuildRequired(shardIndex));
  std::string hashTableFilePath =
      Shard::getStorageHashTableFilePath(shardIndex);
  std::string valuesFilePath = Shard::getValuesFilePath(shardIndex);

  std::vector<Entry> shardEntries =
      StorageHashTable{storage::readFile(hashTableFilePath)}.getEntries();
  std::vector<Entry> cacheMapUpdatedEntries;
  StorageHashTable newStorageHashTable{STORAGE_HASH_TABLE_INITIAL_SIZE};

  std::string newHashTableFilePath = hashTableFilePath + ":rebuilt";
  std::string newValuesFilePath = valuesFilePath + ":rebuilt";
  storage::writeFile(newValuesFilePath, ByteArray{0});
  Storage newValuesStorage{newValuesFilePath};

  for (const auto& shardEntry : shardEntries) {
    const Key& key = shardEntry.key;
    Ptr cacheMapPtr = cacheMap.get(key);
    switch (cacheMapPtr.getType()) {
    case PtrType::DELETED: {
      cacheMapUpdatedEntries.emplace_back(key, Ptr{});
      // cacheMapUpdatedEntries.emplace_back(key, Ptr{PtrType::NONEXISTENT});
      break;
    }
    case PtrType::EMPTY_PTR: {
      if (shardEntry.ptr.getType() == PtrType::PRESENT) {
        size_t newOffset = newValuesStorage.append(
            shard.readValueDirectly(shardIndex, shardEntry.ptr).getBytes());
        newStorageHashTable.put(Entry{key, Ptr{newOffset, true}});
      }
      break;
    }
    case PtrType::PRESENT: {
      assert(shardEntry.ptr.getType() == PtrType::PRESENT);
      size_t newOffset = newValuesStorage.append(
          shard.readValueDirectly(shardIndex, shardEntry.ptr).getBytes());
      newStorageHashTable.put(Entry{key, Ptr{newOffset, true}});
      cacheMapUpdatedEntries.emplace_back(key, Ptr{newOffset, true});
      break;
    }
      /*case PtrType::NONEXISTENT: {
                                 throw std::logic_error("unreachable");
                               }*/
    }
  }
  newValuesStorage.close();
  storage::writeFile(newHashTableFilePath,
                     newStorageHashTable.serializeToByteArray());

  try {
    std::filesystem::rename(newValuesFilePath, valuesFilePath);
    std::filesystem::rename(newHashTableFilePath, hashTableFilePath);
  } catch (const std::exception& exc) {
    throw KVSException{
        KVSErrorType::SHARD_REBUILDER_FAILED_TO_REPLACE_OLD_FILES};
  }

  Shard newShard{newStorageHashTable.getEntries()};
  assert(!newShard.isRebuildRequired(shardIndex));
  return std::make_pair(newShard, cacheMapUpdatedEntries);
}
} // namespace kvs::shard
