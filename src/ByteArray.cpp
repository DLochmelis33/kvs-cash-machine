#include "ByteArray.h"

#include <cassert>

namespace kvs::utils {

ByteArray::ByteArray(size_t length) noexcept : data{std::string(length, '\0')} {
  assert(length > 0);
}

const char* ByteArray::charPtrData() const noexcept { return &data[0]; }

char* ByteArray::charPtrData() noexcept { return &data[0]; }

size_t ByteArray::length() const noexcept { return data.size(); }

} // namespace kvs::utils
