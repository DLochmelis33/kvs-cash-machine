#include "KeyValueTypes.h"
#include "ByteArray.h"
#include "KVSException.h"

#include <cassert>
#include <cstring>

namespace kvs::utils {

Key::Key(ByteArray byteArray) noexcept : bytes{byteArray} {
  assert(byteArray.length() == KEY_SIZE);
}

ByteArray& Key::getBytes() noexcept { return bytes; }

const ByteArray& Key::getBytes() const noexcept { return bytes; }

bool Key::operator==(const Key& other) const noexcept {
  bool res = memcmp(bytes.get(), other.bytes.get(), KEY_SIZE) == 0;
  return res;
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

hash_t hashKey(const Key& key, seed_t seed) noexcept {
  // RV is guaranteed to be equivalent to uint64_t
  return XXH3_64bits_withSeed(key.getBytes().get(), KEY_SIZE, seed);
}

// ----- Ptr impl -----

Ptr::Ptr(ptr_t ptr_) noexcept : ptr(ptr_) {
  static_assert((EMPTY_PTR_V & CONTROL_MASK) == false);
  static_assert((NONEXISTENT_V & CONTROL_MASK) == false);
  // static_assert((SYNC_DELETED_V & CONTROL_MASK) == false);
}

Ptr::Ptr(size_t offset, bool isPresent) noexcept {
  assert(offset % VALUE_SIZE == 0);
  size_t index = offset / VALUE_SIZE;
  assert(index <= 120); // all above are reserved
  ptr = index;
  setValuePresent(isPresent);
}

Ptr::Ptr(PtrType type) noexcept {
  switch (type) {
  case PtrType::EMPTY_PTR:
    ptr = EMPTY_PTR_V;
    break;
  case PtrType::NONEXISTENT:
    ptr = NONEXISTENT_V;
    break;
    // case PtrType::SYNC_DELETED:
    //   ptr = SYNC_DELETED_V;
    break;
  default:
    assert(false && "cannot create non-special ptr without a value");
  }
}

size_t Ptr::getIndex() const noexcept { return ptr & ~CONTROL_MASK; }

size_t Ptr::getOffset() const noexcept {
  return getIndex() * VALUE_SIZE;
}

ptr_t Ptr::getRaw() const noexcept { return ptr; }

bool Ptr::isValuePresent() const noexcept { return ptr & CONTROL_MASK; }

void Ptr::setValuePresent(bool isPresent) noexcept {
  assert(getType() == PtrType::PRESENT || getType() == PtrType::DELETED);
  if (isPresent) {
    ptr |= CONTROL_MASK;
  } else {
    ptr &= ~CONTROL_MASK;
  }
}

PtrType Ptr::getType() const noexcept {
  if (ptr == EMPTY_PTR_V)
    return PtrType::EMPTY_PTR;
  if (ptr == NONEXISTENT_V)
    return PtrType::NONEXISTENT;

  if (isValuePresent())
    return PtrType::PRESENT;
  else
    return PtrType::DELETED;
}

bool Ptr::operator==(const Ptr& other) const noexcept {
  return ptr == other.ptr;
}

bool Ptr::operator!=(const Ptr& other) const noexcept {
  return ptr != other.ptr;
}

// ----- Entry impl -----

bool Entry::operator==(const Entry& other) const noexcept {
  return (key == other.key) && (ptr == other.ptr);
}

} // namespace kvs::utils
