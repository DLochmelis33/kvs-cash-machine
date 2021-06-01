#pragma once

#include "KeyValueTypes.h"
#include "Storage.h"
#include <optional>
#include <vector>

/**
 * @brief The hash table that is stored on disk.
 *
 */
class StorageHashTable final {
public:
  explicit StorageHashTable(ByteArray serializedStorageHashTable) noexcept;

  ByteArray serializeToByteArray() const noexcept;

  /**
     * @brief Find the Ptr associated with the key.
     *
     * @param key
     * @return std::optional<Ptr>
     */
  std::optional<Ptr> get(Key key) const noexcept;

  /**
     * @brief Put the <key, ptr> entry into the table.
     *
     * @param entry
     */
  void put(Entry entry);

  std::vector<Entry> getEntries() const noexcept;

private:
  void expand();

private:
  std::vector<Entry> data;
  std::size_t size;
};
