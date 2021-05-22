#pragma once

#include "KeyValueTypes.h"
#include "Storage.h"
#include <optional>

class StorageHashTable {
  public:
    explicit StorageHashTable(const Storage& storage);

    /**
     * @brief Find the Ptr associated with the @key.
     *
     * @param key
     * @return std::optional<Ptr>
     */
    std::optional<Ptr> get(Key key);

  private:
    const Storage& storage;
};