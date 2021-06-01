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
     */
  void add(KeyValue keyValue);

  /**
     * @brief Remove the Key and associated Value from storage. Lazy operation.
     * 
     * Call shard rebuilding after removal if necessary.
     *
     */
  void remove(Key key);

  /**
     * @brief Get the Value associated with the Key.
     *
     * @return The Value associated with the Key or nothing, if no such Value is present.
     */
  std::optional<Value> get(Key key);

  /**
     * @brief Clear the storage.
     *
     */
  void clear();

private:

   /**
    * @brief Rebuilds the shard with the given index.
    * 
    */
   void rebuildShard(size_t shardIndex);


private:

   /**
    * @brief Cache map, stored in RAM.
    * 
    */
  CacheMap cacheMap;

  /**
   * @brief Shard objects representing... shards?
   * 
   */
  std::vector<Shard> shards;
};
