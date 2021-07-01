#include "Shard.h"
#include "Storage.h"
#include "StorageHashTable.h"

#include <filesystem>

using kvs::storage::Storage, kvs::storage_hash_table::StorageHashTable;

namespace kvs::shard {

std::string Shard::storageDirectoryPath = STORAGE_DIRECTORY_PATH;

shard_index_t Shard::getShardIndex(const Key& key) noexcept {
  return hashKey(key) % SHARD_NUMBER;
}

std::pair<Entry, std::optional<Value>>
Shard::readValue(shard_index_t shardIndex, const Key& key) const {
  if (!filter.checkExist(key)) {
    return std::make_pair(Entry{key}, std::optional<Value>{});
  }
  StorageHashTable storageHashTable{
      storage::readFile(getStorageHashTableFilePath(shardIndex))};
  Ptr ptr = storageHashTable.get(key);
  switch (ptr.getType()) {
  case PtrType::EMPTY_PTR:
    return std::make_pair(Entry{key}, std::optional<Value>{});
  case PtrType::DELETED:
    return std::make_pair(Entry{key, ptr}, std::optional<Value>{});
  case PtrType::PRESENT:
    return std::make_pair(Entry{key, ptr},
                          std::optional{readValueDirectly(shardIndex, ptr)});
  case PtrType::NONEXISTENT:
    throw std::logic_error("NONEXISTENT is forbidden in StorageHashTable");
  }
  throw std::logic_error("unreachable");
}

Entry Shard::writeValue(shard_index_t shardIndex, const Key& key,
                        const Value& value) {
  StorageHashTable storageHashTable{
      storage::readFile(getStorageHashTableFilePath(shardIndex))};
  Ptr& ptr = storageHashTable.get(key);
  switch (ptr.getType()) {

  case PtrType::PRESENT: {
    writeValueDirectly(shardIndex, ptr, value);
    return Entry{key, ptr};
  }
  case PtrType::DELETED: {
    writeValueDirectly(shardIndex, ptr, value);
    ++aliveValuesCnt;

    ptr.setValuePresent(true);
    storage::writeFile(getStorageHashTableFilePath(shardIndex),
                       storageHashTable.serializeToByteArray());
    return Entry{key, ptr};
  }
  case PtrType::EMPTY_PTR: {
    Storage storage(getValuesFilePath(shardIndex));
    size_t offset = storage.append(value.getBytes());
    storage.close();
    ++aliveValuesCnt;
    filter.add(key);

    Ptr newPtr{offset, true};
    Entry newEntry{key, newPtr};
    storageHashTable.put(newEntry);
    storage::writeFile(getStorageHashTableFilePath(shardIndex),
                       storageHashTable.serializeToByteArray());

    return newEntry;
  }
  case PtrType::NONEXISTENT: {
    throw std::logic_error("NONEXISTENT is forbidden in StorageHashTable");
  }
  }
  throw std::logic_error("unreachable");
}

Entry Shard::removeEntry(shard_index_t shardIndex, const Key& key) {
  Entry entry = pushRemoveEntry(shardIndex, key);
  if (entry.ptr.getType() == PtrType::PRESENT) {
    decrementAliveValuesCnt();
  }
  return entry;
}

Entry Shard::pushRemoveEntry(shard_index_t shardIndex, const Key& key) {
  if (!filter.checkExist(key)) {
    return Entry{key};
  }
  StorageHashTable storageHashTable{
      storage::readFile(getStorageHashTableFilePath(shardIndex))};
  Ptr& ptr = storageHashTable.get(key);
  switch (ptr.getType()) {
  case PtrType::DELETED:
    return Entry{key, ptr};
  case PtrType::EMPTY_PTR:
    return Entry{key};
  case PtrType::PRESENT: {
    ptr.setValuePresent(false);
    storage::writeFile(getStorageHashTableFilePath(shardIndex),
                       storageHashTable.serializeToByteArray());
    return Entry{key, ptr};
  }
  case PtrType::NONEXISTENT: {
    throw std::logic_error("NONEXISTENT is forbidden in StorageHashTable");
  }
  }
  throw std::logic_error("unreachable");
}

Value Shard::readValueDirectly(shard_index_t shardIndex, Ptr ptr) const {
  Storage storage{getValuesFilePath(shardIndex)};
  Value value{storage.read(ptr.getOffset(), VALUE_SIZE)};
  storage.close();
  return value;
}

void Shard::writeValueDirectly(shard_index_t shardIndex, Ptr ptr,
                               const Value& value) {
  Storage storage{getValuesFilePath(shardIndex)};
  storage.write(ptr.getOffset(), value.getBytes());
  storage.close();
}

void Shard::incrementAliveValuesCnt() noexcept { ++aliveValuesCnt; }

void Shard::decrementAliveValuesCnt() noexcept { --aliveValuesCnt; }

bool Shard::isRebuildRequired(shard_index_t shardIndex) const noexcept {
  values_cnt_t valuesCnt =
      std::filesystem::file_size(getValuesFilePath(shardIndex)) / VALUE_SIZE;
  return valuesCnt * MAX_OUTDATED_RECORDS_LOAD_FACTOR > aliveValuesCnt;
}

Shard::Shard() noexcept : aliveValuesCnt{0}, filter{} {}

Shard::Shard(const std::vector<Entry>& storageHashTableEntries) noexcept
    : aliveValuesCnt{static_cast<values_cnt_t>(storageHashTableEntries.size())},
      filter{} {
  for (const Entry& entry : storageHashTableEntries) {
    filter.add(entry.key);
  }
}

std::string Shard::getShardDirectoryPath(shard_index_t shardIndex) noexcept {
  return Shard::storageDirectoryPath + std::to_string(shardIndex);
}

std::string Shard::getValuesFilePath(shard_index_t shardIndex) noexcept {
  return Shard::storageDirectoryPath + std::to_string(shardIndex) + "/values";
}

std::string
Shard::getStorageHashTableFilePath(shard_index_t shardIndex) noexcept {
  return Shard::storageDirectoryPath + std::to_string(shardIndex) + "/index";
}

//  values_cnt_t aliveValuesCnt;
//  BloomFilter filter;

} // namespace kvs::shard
