#pragma once

#include "KeyValueTypes.h"
#include <optional>

/**
 * @brief Cache map. Is stored in RAM.
 * 
 */
class CacheMap {

    using Entry = std::pair<Key, Ptr>;

  public:

    explicit CacheMap();
    ~CacheMap();

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
     * @return std::optional<Ptr>
     */
    std::optional<Ptr> get(Key key);

    /**
     * @brief Clear the entire map.
     *
     */
    void clear();

  private:
    char* data;
    size_t size;
};