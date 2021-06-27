#include "KeyValueTypes.h"
#include "KVSException.h"

#include <cassert>

namespace kvs::utils {

Key::Key(key_t key_) noexcept : key(key_) {}

key_t Key::get() const noexcept { return key; }

bool Key::operator==(const Key& other) const noexcept {
  return key == other.key;
}

Value::Value(ByteArray bytes_) : bytes{std::move(bytes_)} {}

const ByteArray& Value::getBytes() const noexcept { return bytes; }

bool Value::operator==(const Value& other) const noexcept {
  if (bytes.length() != other.bytes.length()) {
    return false;
  }
  const char* charBytes = bytes.get();
  const char* otherCharBytes = other.bytes.get();
  for (size_t i = 0, size = bytes.length(); i < size; ++i) {
    if (charBytes[i] != otherCharBytes[i]) {
      return false;
    }
  }
  return true;
}

hash_t hashKey(Key key, seed_t seed) noexcept {
  key_t k = key.get();
  // RV is guaranteed to be equivalent to uint64_t
  return XXH3_64bits_withSeed(&k, sizeof(k), seed);
}

Ptr::Ptr(ptr_t ptr_) noexcept : ptr(ptr_) {
  static_assert((EMPTY_PTR_V & CONTROL_MASK) == false);
}

Ptr::Ptr(size_t offset, bool isPresent) {
  assert(offset % VALUE_SIZE == 0);
  size_t index = offset / VALUE_SIZE;
  assert(index >= EMPTY_PTR_V);
  ptr = index;
  setValuePresent(isPresent);
}

size_t Ptr::get() const noexcept { return ptr & ~CONTROL_MASK; }

size_t Ptr::getOffset() const noexcept {
  return (ptr & ~CONTROL_MASK) * VALUE_SIZE;
}

bool Ptr::isValuePresent() const noexcept { return ptr & CONTROL_MASK; }

void Ptr::setValuePresent(bool isPresent) noexcept {
  if (isPresent) {
    ptr |= CONTROL_MASK;
  } else {
    ptr &= ~CONTROL_MASK;
  }
}

PtrType Ptr::getType() const noexcept {
  if (ptr == EMPTY_PTR_V) {
    return PtrType::EMPTY_PTR;
  }
  if (isValuePresent()) {
    return PtrType::PRESENT;
  }
  return PtrType::DELETED;
}

bool Ptr::operator==(const Ptr& other) noexcept { return ptr == other.ptr; }

bool Ptr::operator!=(const Ptr& other) noexcept { return ptr != other.ptr; }

} // namespace kvs::utils
