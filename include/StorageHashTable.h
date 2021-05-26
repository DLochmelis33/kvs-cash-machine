#pragma once

#include "KeyValueTypes.h"
#include "Storage.h"
#include <optional>

/**
 * @brief The hash table that is stored on disk.
 *
 */
final class StorageHashTable {
public:
  explicit StorageHashTable(const Storage& storage);

  /**
     * @brief Find the Ptr associated with the key.
     *
     * @param key
     * @return std::optional<Ptr>
     */
  std::optional<Ptr> get(Key key) const;

  /**
     * @brief Put the <key, ptr> entry into the table.
     *
     * @param key
     * @param ptr
     */
  void put(Key key, Ptr ptr);

private:
  /**
     * @brief The Storage that is used for reading and writing hash table files on disk.
     *
     */
  const Storage& storage;
};
