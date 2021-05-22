#pragma once

#include "BloomFilter.h"
#include "KeyValueTypes.h"

class Shard {
  public:
    explicit Shard(size_t index);

    /**
     * @brief Get the shard index that contains the @key.
     *
     * @param key
     * @return Shard index.
     */
    static size_t getShardIndex(Key key);

    /**
     * @brief Read a value directly from storage. Used when CacheMap is hit.
     *
     * @param ptr
     * @return Value
     */
    Value readValueDirectly(Ptr ptr) const;

    /**
     * @brief
     *
     * @param key
     * @return std::optional<Value>
     */
    std::optional<Value> readValue(Key key);

    /**
     * @brief
     *
     * @param ptr
     * @param value
     */
    void writeValueDirectly(Ptr ptr, const Value& value);

    /**
     * @brief
     *
     * Used when a deleted element in CacheMap is hit.
     *
     */
    void incrementAliveValuesCnt();

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

    BloomFilter<Key> filter;
};
