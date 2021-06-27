#include "StorageHashTable.h"
#include "KVSException.h"
#include <cassert>
#include <cstring>
#include <functional>

namespace kvs::storage_hash_table {

StorageHashTable::StorageHashTable(const ByteArray array) {
  size_t entrySize = KEY_SIZE + sizeof(ptr_t);
  size_t withoutUsedSize = array.length() - sizeof(size_t);
  if (withoutUsedSize % entrySize != 0)
    throw KVSException(KVSErrorType::TABLE_INVALID_BUILD_DATA);
  size_t dataSize = withoutUsedSize / entrySize;
  data.resize(dataSize);

  for (size_t i = 0; i < dataSize; i++) {
    Key key;
    memcpy(key.get(), array.get() + (i * entrySize), KEY_SIZE);
    ptr_t p =
        *reinterpret_cast<const ptr_t*>(array.get() + i * entrySize + KEY_SIZE);
    data[i] = Entry(key, Ptr(p));
  }
  usedSize = *reinterpret_cast<const size_t*>(array.get() + withoutUsedSize);
}

ByteArray StorageHashTable::serializeToByteArray() const noexcept {
  constexpr size_t entrySize = KEY_SIZE + sizeof(ptr_t);
  size_t resultLength = data.size() * entrySize + sizeof(size_t);
  ByteArray result(resultLength);

  for (size_t i = 0; i < data.size(); i++) {
    auto [key, ptr] = data[i];
    memcpy(result.get() + (i * entrySize), key.get(), KEY_SIZE);
    result.get()[i * entrySize + KEY_SIZE] = ptr.getRaw();
  }
  memcpy(result.get() + (resultLength - sizeof(size_t)),
         reinterpret_cast<const char*>(&usedSize), sizeof(size_t));

  return result;
}

StorageHashTable::StorageHashTable(size_t size) noexcept {
  usedSize = 0;
  data.resize(size);
  // default key will be stored with EMPTY_PTR => no collisions in case of real Key(0)
  Key defaultKey;
  for (size_t i = 0; i < size; i++) data[i] = Entry(defaultKey, EMPTY_PTR);
}

/**
 * @brief Find the next entry by predicate \b exclusively after fromIndex.
 * 
 * @return size_t The found index \b or fromIndex, if no entry was found.
 */
size_t findNextByPredicate(const std::vector<Entry>& data, size_t fromIndex,
                           std::function<bool(const Entry&)> predicate) {
  for (size_t i = (fromIndex != data.size() - 1 ? fromIndex + 1 : 0);
       i != fromIndex; i = (i + 1) % data.size()) {
    if (predicate(data[i]))
      return i;
  }
  return fromIndex;
}

void StorageHashTable::put(const Entry& entry) noexcept {
  auto [key, ptr] = entry;
  size_t keyIndex = hashKey(key) % data.size();

  auto entryCheck = [&entry](const Entry& e) {
    return e.ptr == EMPTY_PTR || // either no such key before and new empty spot
           e.key == entry.key; // or there is such key
  };

  if (!entryCheck(data[keyIndex])) {
    size_t newIndex = findNextByPredicate(data, keyIndex, entryCheck);
    assert(keyIndex != newIndex);
    keyIndex = newIndex;
  }
  data[keyIndex] = entry;
  usedSize++;

  if (usedSize * MAP_LOAD_FACTOR > data.size())
    expand();
}

Ptr& StorageHashTable::get(const Key& key) noexcept {
  size_t keyIndex = hashKey(key) % data.size();
  if (data[keyIndex].key == key)
    return data[keyIndex].ptr;
  size_t newIndex = findNextByPredicate(
      data, keyIndex, [&key](Entry e) { return e.key == key; });
  if (newIndex == keyIndex)
    return EMPTY_PTR;
  return data[newIndex].ptr;
}

const std::vector<Entry> StorageHashTable::getEntries() const noexcept {
  std::vector<Entry> result;
  for (auto [key, ptr] : data) {
    if (ptr != EMPTY_PTR)
      result.push_back(Entry(key, ptr));
  }
  return result;
}

void StorageHashTable::expand() {
  size_t newSize = data.size() * TABLE_EXPANSION_FACTOR;
  if (newSize > TABLE_MAX_SIZE)
    throw KVSException(KVSErrorType::SHARD_OVERFLOW);

  std::vector<Entry> oldData = getEntries();
  data.clear();
  usedSize = 0;
  data.resize(newSize);
  for (Entry e : oldData) put(e);
}

} // namespace kvs::storage_hash_table
