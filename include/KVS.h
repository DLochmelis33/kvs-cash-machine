#pragma once

#include "CacheMap.h"
#include "KeyValueTypes.h"
#include "Shard.h"
#include <optional>
#include <vector>

/**
 * @brief The class that provides an access to a key-value storage.
 *
 */
class KVS {

  public:
    KVS();

    /**
     * @brief Add a KeyValue to the storage.
     *
     * @param keyValue
     */
    void add(KeyValue keyValue);

    /**
     * @brief Remove the Key and associated Value from storage.
     *
     * @param key
     */
    void remove(Key key);

    /**
     * @brief Get the Value associated with the Key.
     *
     * @param key
     * @return The Value associated with the Key or nothing, if none.
     */
    std::optional<Value> get(Key key);

    /**
     * @brief Clear the storage.
     *
     */
    void clear();

  private:
    using hash_t = uint64_t;
    /**
     * @brief Hash the Key.
     *
     * @param key
     * @return
     */
    hash_t hash(Key key);

  private:
    CacheMap cacheMap;
    std::vector<Shard> shards;
};
