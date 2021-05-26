#pragma once

#include <exception>

enum class KVSErrorType { 
  SHARD_OVERFLOW, IO_ERROR  // TODO: different types of io errors
};

class KVSException final : public std::exception {
public:
  explicit KVSException(KVSErrorType errType) noexcept;
  const char* what() const noexcept override;

private:
  KVSErrorType errType;
};
