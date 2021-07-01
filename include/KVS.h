#pragma once

#include "CacheMap.h"
#include "KeyValueTypes.h"
#include "Shard.h"
#include <optional>
#include <vector>

namespace kvs {

using namespace utils;
using kvs::cache_map::CacheMap;
using kvs::shard::Shard;

/**
 * @brief The class that provides an access to a key-value storage.
 *
 */
class KVS final {

public:
  KVS();

  /**
     * @brief Add a new record to the storage.
     *
     */
  void add(const Key& key, const Value& value);

  /**
     * @brief Remove the Key and associated Value from storage. Lazy operation.
     * 
     * Call shard rebuilding after removal if necessary.
     *
     */
  void remove(const Key& key);

  /**
     * @brief Get the Value associated with the Key.
     *
     * @return The Value associated with the Key or nothing, if no such Value is present.
     */
  std::optional<Value> get(const Key& key);

  /**
     * @brief Clear the storage entirely.
     *
     */
  void clear();

private:
  /**
    * @brief Rebuilds the shard with the given index.
    * 
    */
  void rebuildShard(shard_index_t shardIndex);

private:
  /**
   * @brief Shard objects representing... shards?
   * 
   */
  std::vector<Shard> shards;

  /**
    * @brief Cache map, stored in RAM.
    * 
    */
  CacheMap cacheMap;

  /**
    * @brief When an entry is displaced from the CacheMap, the corresponding operation should be pushed to the shard. Use this method for that.
    * 
    * @param displaced 
    */
  void pushOperation(Entry displaced);
};

} // namespace kvs