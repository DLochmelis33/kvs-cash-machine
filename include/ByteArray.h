#pragma once

#include <string>

namespace kvs::utils {

/**
 * @brief A wrapper for an array and its length.
 * 
 */
class ByteArray final {
public:
  /**
   * @brief Create a new ByteArray. 
   * 
   * Zero length is acceptable: the pointer returned from get() methods will be valid, but dereferencing it will cause UB.
   * 
   */
  explicit ByteArray(size_t length) noexcept;

  const char* get() const noexcept;

  char* get() noexcept;

  size_t length() const noexcept;

private:
  std::string data;
};

} // namespace kvs::utils
