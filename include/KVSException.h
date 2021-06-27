#pragma once

#include <exception>

namespace kvs {

enum class KVSErrorType {
  SHARD_OVERFLOW,
  STORAGE_OPEN_FAILED,
  STORAGE_CLOSE_FAILED,
  STORAGE_READ_FAILED,
  STORAGE_WRITE_FAILED,
  PTR_INDEX_OUT_OF_BOUNDS,
  TABLE_INVALID_BUILD_DATA
};

class KVSException final : public std::exception {
public:
  explicit KVSException(KVSErrorType errType) noexcept;
  const char* what() const noexcept override;

private:
  KVSErrorType errType;
};

} // namespace kvs
