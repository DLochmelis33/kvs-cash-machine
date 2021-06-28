#include "CacheMap.h"
#include <cassert>
#include <functional>
#include <random>

namespace kvs::cache_map {

CacheMap::CacheMap(size_t size) noexcept {
  data.resize(size);
  usedSize = 0;
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

std::optional<Entry> CacheMap::putOrDisplace(Entry entry) noexcept {
  auto [key, ptr] = entry;
  size_t keyIndex = hashKey(key) % data.size();

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<size_t> distr;

  std::optional<Entry> displaced;
  if (usedSize * MAP_LOAD_FACTOR > data.size()) {
    // randomly delete one of the entries
    size_t randomIndex = distr(gen) % data.size();
    size_t foundIndex;
    auto emptyCheck = [](const Entry& e) { return e.ptr == EMPTY_PTR; };
    if (data[randomIndex].ptr != EMPTY_PTR) {
      foundIndex = findNextByPredicate(data, randomIndex, emptyCheck);
      assert(foundIndex != randomIndex);
      if (foundIndex == 0)
        foundIndex = data.size() - 1;
      else
        foundIndex -= 1;
    } else {
      foundIndex = randomIndex;
      for (size_t i = randomIndex - 1; i != randomIndex; i--) {
        if (!emptyCheck(data[i])) {
          foundIndex = i;
          break;
        }
        if (i == 0)
          i = data.size();
      }
      assert(foundIndex != randomIndex);
    }
    assert(data[foundIndex].ptr != EMPTY_PTR);
    displaced = std::optional(data[foundIndex]);
    data[foundIndex] = Entry(Key(), Ptr());
    usedSize--;
  }

  // now actually put the Entry into the map
  auto entryCheck = [&entry](const Entry& e) {
    return e.ptr == EMPTY_PTR || // either no such key before and new empty spot
           e.key == entry.key; // or there is such key
  };

  if (!entryCheck(data[keyIndex])) {
    size_t newIndex = findNextByPredicate(data, keyIndex, entryCheck);
    assert(keyIndex != newIndex);
    keyIndex = newIndex;
  }
  if (data[keyIndex].ptr == EMPTY_PTR)
    usedSize++;
  data[keyIndex] = entry;

  return displaced;
}

Ptr CacheMap::get(const Key& key) const noexcept {
  size_t keyIndex = hashKey(key) % data.size();
  if (data[keyIndex].key == key)
    return data[keyIndex].ptr;

  size_t foundIndex =
      findNextByPredicate(data, keyIndex, [&key](const Entry& e) {
        return e.key == key || e.ptr == EMPTY_PTR;
      });
  assert(foundIndex != keyIndex);
  return data[foundIndex].ptr;
}

void CacheMap::clear() noexcept {
  size_t size = data.size();
  data.clear();
  data.resize(size);
  usedSize = 0;
}

} // namespace kvs::cache_map