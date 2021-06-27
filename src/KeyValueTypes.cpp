#include "KeyValueTypes.h"
#include "KVSException.h"
#include "xxh3.h"

namespace kvs::utils {

Key::Key(key_t key_) noexcept : key(key_) {}

key_t Key::get() const noexcept { return key; }

bool Key::operator==(const Key& other) const noexcept {
  return key == other.key;
}

value_t Value::get() const noexcept { return value; }

hash_t hashKey(Key key, seed_t seed) noexcept {
  key_t k = key.get();
  // RV is guaranteed to be equivalent to uint64_t
  return XXH3_64bits_withSeed(&k, sizeof(k), seed);
}

Ptr::Ptr(ptr_t ptr_) noexcept : ptr(ptr_) {
  static_assert((EMPTY_PTR_V & CONTROL_MASK) == false);
}

Ptr::Ptr(size_t index, bool isPresent) {
  if (index >= EMPTY_PTR_V)
    throw KVSException(KVSErrorType::PTR_INDEX_OUT_OF_BOUNDS);
  ptr = index;
  setValuePresent(isPresent);
}

ptr_t Ptr::get() const noexcept { return ptr & ~CONTROL_MASK; }

bool Ptr::isValuePresent() const noexcept { return ptr & CONTROL_MASK; }

void Ptr::setValuePresent(bool isPresent) noexcept {
  if (isPresent) {
    ptr |= CONTROL_MASK;
  } else {
    ptr &= ~CONTROL_MASK;
  }
}

bool Ptr::operator==(const Ptr& other) noexcept { return ptr == other.ptr; }

bool Ptr::operator!=(const Ptr& other) noexcept { return ptr != other.ptr; }

} // namespace kvs::utils
