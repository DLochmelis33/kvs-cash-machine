#pragma once

#include "CacheMap.h"
#include "KeyValueTypes.h"
#include "Shard.h"
#include <utility>
#include <vector>

namespace kvs::shard {

using namespace kvs::utils;

class ShardBuilder final {

public:
  /**
   * @brief Create a Shard object. 
   * 
   * A factory method for consistency.
   * 
   */
  static Shard createShard(shard_index_t shardIndex);

  /**
   * @brief Rebuild the Shard according to delayed removals stored in CacheMap.
   * 
   * The old shard is modified, but it is intended to be removed with a new Shard. The old Shard is disallowed to use after calling this method.
   * 
   * @param shard The Shard to rebuild.
   * @param cacheMap The CacheMap to read information about delayed removals from.
   * @return pair.first - The newly created Shard to replace the old one.
   * @return pair.second - Entries in CacheMap that have to be overwritten, including both entries with new ptrs and old removed entries with EMPTY_PTR.
   */
  static std::pair<Shard, std::vector<Entry>>
  rebuildShard(const Shard& shard, shard_index_t shardIndex,
               const kvs::cache_map::CacheMap& cacheMap);
};

} // namespace kvs::shard
