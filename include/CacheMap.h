#pragma once

#include "KeyValueTypes.h"
#include <optional>
#include <vector>

namespace kvs::cache_map {

using namespace kvs::utils;

/**
 * @brief Cache map. Is stored in RAM.
 *
 */
class CacheMap final {

public:
  CacheMap() noexcept;

  /**
     * @brief Put Ptr by Key. If not enough space, displace a random Entry and return it. If an
     * Entry with the given Key already exists, overwrite it.
     *
     * @return Displaced Entry, if any.
     */
  std::optional<Entry> putOrDisplace(Entry entry) noexcept;

  /**
     * @brief Find a Ptr by Key.
     *
     * @return The requested Ptr or nothing, if no Entry with given Key is present.
     */
  std::optional<Ptr> get(Key key) const noexcept;

  /**
     * @brief Clear the entire map.
     *
     */
  void clear() noexcept;

private:
  /**
    * @brief The internal storage of the map.
    * 
    */
  std::vector<Entry> data;

  /**
     * @brief The number of elements present in the map.
     *
     */
  size_t size;
};

} // namespace kvs::cache_map
