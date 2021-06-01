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
class KVS final {

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
   /**
    * @brief Rebuilds the shard with given index.
    * 
    * @param shardIndex 
    */
   void rebuildShard(size_t shardIndex);


private:
  CacheMap cacheMap;
  std::vector<Shard> shards;
};
