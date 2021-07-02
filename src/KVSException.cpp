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
  case KVSErrorType::PTR_INDEX_OUT_OF_BOUNDS:
    return "Failed to create Ptr object: index is too large";
  case KVSErrorType::STORAGE_HASH_TABLE_INVALID_BUILD_DATA:
    return "Failed to build StorageHashTable: invalid data";
  case KVSErrorType::FAILED_TO_CREATE_SHARD_DIRECTORY:
    return "Failed to create shard directory";
  case KVSErrorType::SHARD_REBUILDER_FAILED_TO_REPLACE_OLD_FILES:
    return "ShardRebuilder failed to move new shard files to old ones";
  case KVSErrorType::FAILED_TO_GET_VALUES_FILE_SIZE:
    return "Failed to get shard values file size";
  }
  return "<unsupported exception type>";
}

} // namespace kvs
