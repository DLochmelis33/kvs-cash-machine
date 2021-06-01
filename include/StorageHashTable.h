#pragma once

#include "KeyValueTypes.h"
#include "Storage.h"
#include <optional>
#include <vector>

/**
 * @brief The hash table that is stored on disk.
 * 
 * Operates in RAM, so can be used with any kind of storage via serializing.
 *
 */
class StorageHashTable final {
public:

  explicit StorageHashTable(ByteArray serializedStorageHashTable) noexcept;

  /**
   * @brief Serialize the table into a ByteArray.
   * 
   */
  ByteArray serializeToByteArray() const noexcept;

  /**
     * @brief Find the Ptr associated with the key.
     *
     * @return The requested Ptr or nothing, if no Entry with given Key is present.
     */
  std::optional<Ptr> get(Key key) const noexcept;

  /**
     * @brief Put an Entry into the table.
     *
     */
  void put(Entry entry);

  /**
   * @brief Get all entries present in the map.
   * 
   * Used during rebuilding.
   * 
   * @return std::vector<Entry> 
   */
  std::vector<Entry> getEntries() const noexcept;

private:
  /**
   * @brief Expand the map to twice its current capacity.
   * 
   * Needs to be called once the number of elements exceeds capacity * a predefined constant. Only works one time; calling this method a second time causes a SHARD_OVERFLOW typed KVSException.
   * 
   */
  void expand();

private:
  std::vector<Entry> data;
  std::size_t size;
};
