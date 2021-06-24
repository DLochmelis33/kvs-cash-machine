#include "KVSException.h"

namespace kvs {

KVSException::KVSException(KVSErrorType errType_) noexcept
    : errType{errType_} {}

const char* KVSException::what() const noexcept {
  switch (errType) {
  case KVSErrorType::SHARD_OVERFLOW:
    return "Shard overflow error";
  case KVSErrorType::STORAGE_OPEN_FAILED:
    return "Failed to open storage file";
  case KVSErrorType::STORAGE_CLOSE_FAILED:
    return "Failed to close storage file";
  case KVSErrorType::STORAGE_READ_FAILED:
    return "Failed to read storage file";
  case KVSErrorType::STORAGE_WRITE_FAILED:
    return "Failed to write to storage file";
  }
  return "<unsupported exception type>";
}

} // namespace kvs
