#pragma once

#include "KeyValueTypes.h"
#include "Storage.h"
#include <optional>
#include <vector>

namespace kvs::storage_hash_table {

using namespace kvs::utils;

/**
 * @brief The hash table that is stored on disk.
 * 
 * Operates in RAM, so can be used with any kind of storage via serializing to ByteArray.
 * 
 * Invariant: no NONEXISTENT Ptr-s are allowed to be stored in this table.
 *
 */
class StorageHashTable final {
public:
  /**
   * @brief Deserialize a StorageHashTable from serializedStorageHashTable.
   * 
   * @throws KVSException if the data is corrupted.
   */
  explicit StorageHashTable(const ByteArray serializedStorageHashTable);

  /**
   * @brief Construct a new empty StorageHashTable.
   * 
   */
  explicit StorageHashTable(size_t size) noexcept;
  /**
   * @brief Serialize the table into a ByteArray.
   * 
   */
  ByteArray serializeToByteArray() const noexcept;

  /**
     * @brief Find the Ptr associated with the key.
     *
     * @return The requested Ptr or EMPTY_PTR, if no Entry with given Key is present.
     */
  Ptr& get(const Key& key) noexcept;
  const Ptr& get(const Key& key) const noexcept;

  /**
     * @brief Put an Entry into the table.
     *
     */
  void put(const Entry& entry) noexcept;

  /**
   * @brief Get all entries \b present (i.e. those which Ptr is not EMPTY_PTR) in the map.
   * 
   * Used during rebuilding.
   * 
   */
  std::vector<Entry> getEntries() const noexcept;

private:
  /**
   * @brief Expand the map to twice its current capacity.
   * 
   * Needs to be called once the number of elements exceeds capacity * a predefined constant. Only works one time; calling this method a second time causes a SHARD_OVERFLOW typed KVSException.
   * 
   * @throws KVSException if the table is already expanded
   */
  void expand();

private:
  std::vector<Entry> data;
  std::size_t usedSize;
};

} // namespace kvs::storage_hash_table
