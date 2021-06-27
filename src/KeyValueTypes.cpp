#include "KeyValueTypes.h"
#include "KVSException.h"
#include "xxh3.h"
#include <cassert>
#include <cstring>

namespace kvs::utils {

Key::Key(key_t key) noexcept : data(KEY_SIZE) {
  memcpy(data.get(), key, KEY_SIZE);
}

Key::Key() noexcept : data(KEY_SIZE) {}

Key::Key(__uint128_t key) noexcept : Key(reinterpret_cast<char*>(&key)) {}

key_t Key::get() noexcept { return data.get(); }

const char* Key::get() const noexcept { return data.get(); }

bool Key::operator==(const Key& other) const noexcept {
  bool res = memcmp(data.get(), other.data.get(), KEY_SIZE) == 0;
  return res;
}

value_t Value::get() const noexcept { return value; }

hash_t hashKey(const Key& key, seed_t seed) noexcept {
  // RV is guaranteed to be equivalent to uint64_t
  return XXH3_64bits_withSeed(key.get(), KEY_SIZE, seed);
}

Ptr::Ptr(ptr_t ptr_) noexcept : ptr(ptr_) {
  static_assert((EMPTY_PTR_V & CONTROL_MASK) == false);
}

Ptr::Ptr(size_t index, bool isPresent) noexcept {
  assert(index < EMPTY_PTR_V);
  ptr = index;
  setValuePresent(isPresent);
}

Ptr::Ptr() noexcept : Ptr(Ptr::EMPTY_PTR_V) {}

ptr_t Ptr::get() const noexcept { return ptr & ~CONTROL_MASK; }

ptr_t Ptr::getRaw() const noexcept { return ptr; }

bool Ptr::isValuePresent() const noexcept { return ptr & CONTROL_MASK; }

void Ptr::setValuePresent(bool isPresent) noexcept {
  if (isPresent) {
    ptr |= CONTROL_MASK;
  } else {
    ptr &= ~CONTROL_MASK;
  }
}

bool Ptr::operator==(const Ptr& other) const noexcept {
  return ptr == other.ptr;
}

bool Ptr::operator!=(const Ptr& other) const noexcept {
  return ptr != other.ptr;
}

Ptr::PtrType Ptr::getType() const noexcept {
  if (ptr == EMPTY_PTR_V) {
    return PtrType::EMPTY_PTR;
  }
  if (isValuePresent()) {
    return PtrType::PRESENT;
  }
  return PtrType::DELETED;
}

bool Entry::operator==(const Entry& other) const noexcept {
  return (key == other.key) && (ptr == other.ptr);
}

} // namespace kvs::utils
