#include "StorageHashTable.h"
#include "KVSException.h"
#include <cassert>
#include <functional>

namespace kvs::storage_hash_table {

StorageHashTable::StorageHashTable(ByteArray array) {
  size_t entrySize = sizeof(key_t) + sizeof(ptr_t);
  size_t withoutUsedSize = array.length() - sizeof(size_t);
  if (withoutUsedSize % entrySize != 0)
    throw KVSException(KVSErrorType::TABLE_INVALID_BUILD_DATA);
  size_t dataSize = withoutUsedSize / entrySize;
  data.resize(dataSize);

  for (size_t i = 0; i < dataSize; i++) {
    key_t k = *reinterpret_cast<key_t*>(array.get() + i * entrySize);
    ptr_t p =
        *reinterpret_cast<ptr_t*>(array.get() + i * entrySize + sizeof(key_t));
    data[i] = Entry(Key(k), Ptr(p));
  }
  usedSize = *reinterpret_cast<size_t*>(array.get() + withoutUsedSize);
}

ByteArray StorageHashTable::serializeToByteArray() const noexcept {
  size_t entrySize = sizeof(key_t) + sizeof(ptr_t);
  size_t resultLength = data.size() * entrySize + sizeof(size_t);
  ByteArray result(resultLength);

  for (size_t i = 0; i < data.size(); i++) {
    auto [key, ptr] = data[i];
    *reinterpret_cast<key_t*>(result.get() + i * entrySize) = key.get();
    *reinterpret_cast<ptr_t*>(result.get() + i * entrySize + sizeof(key_t)) =
        ptr.get();
  }
  *reinterpret_cast<size_t*>(result.get()) = usedSize;
  return result;
}

StorageHashTable::StorageHashTable(size_t size) noexcept {
  usedSize = 0;
  data.resize(size);
  // default key will be stored with EMPTY_PTR => no collisions in case of real Key(0)
  Key defaultKey(0);
  for (size_t i = 0; i < size; i++) data[i] = Entry(defaultKey, EMPTY_PTR);
}

/**
 * @brief Find the next entry by predicate \b exclusively after fromIndex.
 * 
 * @return size_t The found index \b or fromIndex, if no entry was found.
 */
size_t findNextByPredicate(const std::vector<Entry>& data, size_t fromIndex,
                           const std::function<bool(const Entry&)> predicate) {
  for (size_t i = (fromIndex != data.size() - 1 ? fromIndex + 1 : 0);
       i != fromIndex; i = (i + 1) % data.size()) {
    if (predicate(data[i]))
      return i;
  }
  return fromIndex;
}

void StorageHashTable::put(Entry entry) noexcept {
  auto [key, ptr] = entry;
  size_t keyIndex = hashKey(key) % data.size();
  if (data[keyIndex].ptr != EMPTY_PTR) {
    size_t newIndex = findNextByPredicate(
        data, keyIndex, [](Entry e) { return e.ptr == EMPTY_PTR; });
    assert(keyIndex != newIndex);
    keyIndex = newIndex;
  }
  data[keyIndex] = entry;
  usedSize++;
}

Ptr StorageHashTable::get(Key key) const noexcept {
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
  return data;
}

void StorageHashTable::expand() {
  std::vector<Entry> oldData = data;
  size_t newSize = data.size() * TABLE_EXPANSION_FACTOR;
  if(newSize > TABLE_MAX_SIZE)
    throw KVSException(KVSErrorType::SHARD_OVERFLOW);
  data.clear();
  data.resize(newSize);
  std::vector<Entry> newData(data.size() * TABLE_EXPANSION_FACTOR);
  for (Entry e : oldData) put(e);
}

} // namespace kvs::storage_hash_table
