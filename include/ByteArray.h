#pragma once

#include <string>

namespace kvs::utils {

/**
 * @brief A wrapper for an array and its length.
 * 
 */
// TODO docs: empty length is ok, get methods also, but then ptr must not be deref
class ByteArray final {
public:
  explicit ByteArray(size_t length) noexcept;

  const char* get() const noexcept;

  char* get() noexcept;

  size_t length() const noexcept;

private:
  std::string data;
};

} // namespace kvs::utils
