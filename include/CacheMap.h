#pragma once

#include "KeyValueTypes.h"
#include <optional>
#include <vector>

/**
 * @brief Cache map. Is stored in RAM.
 *
 */
final class CacheMap {

  using Entry = std::pair<Key, Ptr>;

public:
  CacheMap();

  /**
     * @brief Put @ptr by @key. If not enough space, displace a random entry and return it. If an
     * entry with the given key already exists, overwrite it.
     *
     * @param entry
     * @return displaced entry, if any
     */
  std::optional<Entry> putOrDisplace(Entry entry);

  /**
     * @brief Find a @ptr by @key.
     *
     * @param key
     * @return
     */
  std::optional<Ptr> get(Key key) const;

  /**
     * @brief Clear the entire map.
     *
     */
  void clear();

private:
  std::vector<Entry> data;
  /**
     * @brief The number of elements present in the map.
     *
     */
  size_t size;
};
