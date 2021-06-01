#pragma once

#include "BloomFilter.h"
#include "KeyValueTypes.h"

/**
 * @brief A single part of KVS after sharding. Contains a BloomFilter and loads hash tables into RAM from disk when necessary.
 * 
 * Instantiated via ShardBuilder class static method.
 *
 */
class Shard final {
public:
  Shard() = delete;

  /**
     * @brief Get the index of a shard that contains the Key.
     *
     */
  static size_t getShardIndex(Key key) noexcept;

  /**
     * @brief Read a value from this shard.
     *
     * @return pair.first - the Entry corresponding to the Key. Used to update the CacheMap.
     * @return pair.second - the actual Value or nothing, if none is present.
     */
  std::pair<Entry, std::optional<Value>> readValue(Key key) const;

  /**
     * @brief Write a Value to this shard.
     *
     * @return Either the old or the new Entry that corresponds to given Key.
     */
  Entry writeValue(Key key, const Value& value);

  /**
     * @brief Remove a Value from this shard.
     *
     * Can also handle delayed removals.
     *
     * @return The new Entry that corresponds to given Key.
     */
  Entry removeEntry(Key key);

  /**
     * @brief Read a Value directly from disk storage. Used when CacheMap entry is hit.
     *
     * Attention: this nethod does not update the internal alive values counter, it has to be done using the increment / decrement methods.
     *
     * @return The read Value. If the Ptr points to a nonexistent Value, the behavior is undefined.
     */
  Value readValueDirectly(Ptr ptr) const;

  /**
     * @brief Write a Value directly to disk storage. Used when CacheMap entry is hit.
     *
     * Attention: this nethod does not update the internal alive values counter, it has to be done using the increment / decrement methods.
     *
     */
  void writeValueDirectly(Ptr ptr, const Value& value);

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
  bool isRebuildRequired() const noexcept;

private:
  explicit Shard(size_t index) noexcept;
  explicit Shard(size_t index,
                 const std::vector<Entry>& storageHashTableEntries) noexcept;

  /**
     * @brief Shard index.
     *
     */
  size_t index;

  /**
     * @brief Number of values that are stored on disk, but not deleted yet.
     *
     */
  size_t aliveValuesCnt;

  /**
     * @brief A filter.
     *
     */
  BloomFilter filter;

  friend class ShardBuilder;
};
