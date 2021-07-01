#pragma once

#include "BloomFilter.h"
#include "KeyValueTypes.h"

#include <optional>
#include <string>

namespace kvs::shard {

using namespace kvs::utils;

/**
 * @brief A single part of KVS after sharding. Contains a BloomFilter and loads hash tables into RAM from disk when necessary.
 * 
 * Instantiated via ShardBuilder class static method.
 *
 */
class Shard final {
public:
  /**
     * @brief Get the index of a shard that contains the Key.
     *
     */
  static shard_index_t getShardIndex(const Key& key) noexcept;

  /**
     * @brief Read a value from this shard.
     *
     * @return pair.first - the Entry corresponding to the Key. Used to update the CacheMap. \n pair.second - the actual Value or nothing, if none is present.
     */
  std::pair<Entry, std::optional<Value>> readValue(shard_index_t shardIndex,
                                                   const Key& key) const;

  /**
     * @brief Write a Value to this shard.
     *
     * @return Either the old or the new Entry that corresponds to given Key.
     */
  Entry writeValue(shard_index_t shardIndex, const Key& key,
                   const Value& value);

  /**
     * @brief Remove a Value from this shard.
     *
     * Can also handle delayed removals.
     *
     * @return The new Entry that corresponds to given Key.
     */
  Entry removeEntry(shard_index_t shardIndex, const Key& key);

  // TODO docs
  Entry pushRemoveEntry(shard_index_t shardIndex, const Key& key);

  /**
     * @brief Read a Value directly from disk storage. Used when CacheMap entry is hit.
     *
     * Attention: this nethod does not update the internal alive values counter, it has to be done using the increment / decrement methods.
     *
     * @return The read Value. If the Ptr points to a nonexistent Value, the behavior is undefined.
     */
  Value readValueDirectly(shard_index_t shardIndex, Ptr ptr) const;

  /**
     * @brief Write a Value directly to disk storage. Used when CacheMap entry is hit.
     *
     * Attention: this nethod does not update the internal alive values counter, it has to be done using the increment / decrement methods.
     *
     */
  void writeValueDirectly(shard_index_t shardIndex, Ptr ptr,
                          const Value& value);

  /**
     * @brief Increase the counter of non-deleted elements in this shard.
     *
     * Used by KVS when a deleted element in CacheMap is added again.
     *
     */
  void incrementAliveValuesCnt() noexcept;

  /**
    * @brief Decrease the counter of non-deleted elements in this shard.
    *
    * Used by KVS when an entry in CacheMap is marked as deleted.
    * 
    */
  void decrementAliveValuesCnt() noexcept;

  /**
    * @brief Check if a rebuild needs to be called.
    * 
    */
  bool isRebuildRequired(shard_index_t shardIndex) const noexcept;

  // TODO docs
  static std::string storageDirectoryPath; // = STORAGE_DIRECTORY_PATH

  static std::string getShardDirectoryPath(shard_index_t shardIndex) noexcept;

  static std::string getValuesFilePath(shard_index_t shardIndex) noexcept;

  static std::string
  getStorageHashTableFilePath(shard_index_t shardIndex) noexcept;

private:
  // TODO docs: description
  explicit Shard() noexcept;
  explicit Shard(const std::vector<Entry>& storageHashTableEntries) noexcept;

  /**
     * @brief Number of values that are stored on disk, but not deleted yet.
     *
     */
  values_cnt_t aliveValuesCnt;

  /**
     * @brief A filter.
     *
     */
  bloom_filter::BloomFilter filter;

  friend class ShardBuilder;
};

} // namespace kvs::shard
