#pragma once

#include "KeyValueTypes.h"
#include "Storage.h"
#include "Shard.h"
#include "CacheMap.h"
#include <utility>
#include <vector>

class ShardBuilder final {

public:
  /**
   * @brief Create a Shard object. 
   * 
   * A factory method for consistency.
   * 
   */
  static Shard createShard(size_t index);

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
      rebuildShard(const Shard& shard, const CacheMap& cacheMap);
};
