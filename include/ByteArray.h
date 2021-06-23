#pragma once

#include <string>

namespace kvs::utils {

/**
 * @brief A wrapper for an array and its size.
 * 
 */
class ByteArray final {
public:
  explicit ByteArray(size_t length) noexcept;

  const char* charPtrData() const noexcept;

  char* charPtrData() noexcept;

  size_t length() const noexcept;

private:
  std::string data;
};

} // namespace kvs::utils
