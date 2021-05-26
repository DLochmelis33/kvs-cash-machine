#pragma once

#include "BloomFilter.h"
#include "KeyValueTypes.h"

/**
 * @brief A single part of KVS after sharding.
 *
 */
class Shard final {
public:
  Shard() = delete;

  /**
     * @brief Get the shard index that contains the Key.
     *
     * @param key
     * @return Shard index.
     */
  static size_t getShardIndex(Key key) noexcept;

  /**
     * @brief Read a value from this shard.
     *
     * @param key
     * @return Entry that corresponds to given Key
     */
  Entry readValue(Key key) const;

  /**
     * @brief Write a value to this shard.
     *
     * @param key
     * @return either the old or new Entry that corresponds to given Key
     */
  Entry writeValue(Key key, const Value& value);

  /**
     * @brief Remove a value from this shard.
     *
     * Can also handle delayed removals.
     *
     * @param key
     * @return Entry that corresponds to given Key
     */
  Entry removeEntry(Key key);

  /**
     * @brief Read a value directly from disk storage. Used when CacheMap entry is hit.
     *
     * Attention: this nethod does not update the internal alive values counter, it has to be done using the increment / decrement methods.
     *
     * @param ptr
     * @return
     */
  Value readValueDirectly(Ptr ptr) const;

  /**
     * @brief Write a Value directly to disk storage. Used when CacheMap entry is hit.
     *
     * Attention: this nethod does not update the internal alive values counter, it has to be done using the increment / decrement methods.
     *
     * @param ptr
     * @param value
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
    * @return boolean 
    */
  boolean isRebuildRequired() const noexcept;

private:
  explicit Shard(size_t index) noexcept;

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
  BloomFilter<Key> filter;

  friend ShardBuilder;
};
