#include "ShardBuilder.h"
#include "Storage.h"
#include "StorageHashTable.h"

#include <filesystem>
#include <iostream>

using kvs::storage::Storage, kvs::storage_hash_table::StorageHashTable;

namespace kvs::shard {

Shard ShardBuilder::createShard(shard_index_t shardIndex) {
  std::filesystem::create_directories(Shard::getShardDirectoryPath(shardIndex));
  storage::writeFile(Shard::getValuesFilePath(shardIndex), ByteArray{0});
  storage::writeFile(
      Shard::getStorageHashTableFilePath(shardIndex),
      StorageHashTable{STORAGE_HASH_TABLE_INITIAL_SIZE}.serializeToByteArray());
  return Shard{};
}

} // namespace kvs::shard
