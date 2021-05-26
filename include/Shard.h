#pragma once

#include "BloomFilter.h"
#include "KeyValueTypes.h"

/**
 * @brief A single part of KVS after sharding.
 *
 */
final class Shard {
public:
  explicit Shard(size_t index);

  /**
     * @brief Get the shard index that contains the Key.
     *
     * @param key
     * @return Shard index.
     */
  static size_t getShardIndex(Key key);

  /**
     * @brief Read a value directly from disk storage. Used when CacheMap entry is hit.
     *
     * @param ptr
     * @return
     */
  Value readValueDirectly(Ptr ptr) const;

  /**
     * @brief Read a value from this shard.
     *
     * @param key
     * @return
     */
  std::optional<Value> readValue(Key key) const;

  /**
     * @brief Write a Value directly to disk storage. Used when CacheMap entry is hit.
     *
     * @param ptr
     * @param value
     */
  void writeValueDirectly(Ptr ptr, const Value& value);

  /**
     * @brief Write a value to this shard.
     *
     * @param key
     * @return
     */
  std::optional<Value> writeValue(Key key, const Value& value);

  /**
     * @brief Increase the counter of non-deleted elements in this shard.
     *
     * Used when a deleted element in CacheMap is hit.
     *
     */
  void incrementAliveValuesCnt();

  // TODO: remove(Key) should be able to tell KVS to perform a cleanup

private:
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
};
