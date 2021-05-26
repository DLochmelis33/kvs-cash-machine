#pragma once

#include "KeyValueTypes.h"
#include "Storage.h"
#include <vector>
#include <utility>

final class ShardBuilder {

  public:

  /**
   * @brief Create a Shard object.
   * 
   * @param index 
   * @return Shard 
   */
  static Shard createShard(size_t index);

  /**
   * @brief Rebuild the shard according to delayed removals stored in CacheMap.
   * 
   * @param shard 
   * @param cacheMap 
   * @return std::vector<Entry> entries in CacheMap that have to be overwritten, including both entries with new ptrs and old removed entries with EMPTY_PTR
   */
  static std::pair<Shard, std::vector<Entry>> rebuildShard(const Shard& shard, const CacheMap& cacheMap);

};