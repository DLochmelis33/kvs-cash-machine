#include "Storage.h"
#include "KVSException.h"

#include <algorithm>
#include <filesystem>

namespace kvs::storage {

ByteArray readFile(std::string filename) {
  std::ifstream file(filename, std::ios::in | std::ios::binary);
  try {
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  } catch (const std::exception& exc) {
    throw KVSException(KVSErrorType::STORAGE_OPEN_FAILED);
  }

  size_t fileSize = std::filesystem::file_size(std::filesystem::path(filename));
  ByteArray bytes(fileSize);
  try {
    file.read(bytes.get(), fileSize);
  } catch (const std::exception& exc) {
    throw KVSException(KVSErrorType::STORAGE_READ_FAILED);
  }
  try {
    file.close();
  } catch (const std::exception& exc) {
    throw KVSException(KVSErrorType::STORAGE_CLOSE_FAILED);
  }
  return bytes;
}

void writeFile(std::string filename, ByteArray bytes) {
  std::ofstream file(filename,
                     std::ios::out | std::ios::binary | std::ios::trunc);
  try {
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  } catch (const std::exception& exc) {
    throw KVSException(KVSErrorType::STORAGE_OPEN_FAILED);
  }
  try {
    file.write(bytes.get(), bytes.length());
  } catch (const std::exception& exc) {
    throw KVSException(KVSErrorType::STORAGE_WRITE_FAILED);
  }
  try {
    file.close();
  } catch (const std::exception& exc) {
    throw KVSException(KVSErrorType::STORAGE_CLOSE_FAILED);
  }
}

Storage::Storage(std::string filename)
    : file{filename, std::ios::out | std::ios::in | std::ios::binary},
      fileSize{std::filesystem::file_size(std::filesystem::path(filename))} {
  try {
    file.exceptions(std::fstream::failbit | std::fstream::badbit);
  } catch (const std::exception& exc) {
    throw KVSException(KVSErrorType::STORAGE_OPEN_FAILED);
  }
}

void Storage::close() {
  try {
    file.close();
  } catch (const std::exception& exc) {
    throw KVSException(KVSErrorType::STORAGE_CLOSE_FAILED);
  }
}

ByteArray Storage::read(size_t offset, size_t length) {
  ByteArray bytes(length);
  try {
    file.seekg(offset);
    file.read(bytes.get(), length);
  } catch (const std::exception& exc) {
    throw KVSException(KVSErrorType::STORAGE_READ_FAILED);
  }
  return bytes;
}

void Storage::write(size_t offset, ByteArray bytes) {
  try {
    file.seekp(offset);
    file.write(bytes.get(), bytes.length());
  } catch (const std::exception& exc) {
    throw KVSException(KVSErrorType::STORAGE_WRITE_FAILED);
  }
  fileSize = std::max(fileSize, offset + bytes.length());
}

size_t Storage::append(ByteArray bytes) {
  size_t prevFileSize = fileSize;
  try {
    file.seekp(0, file.end);
    file.write(bytes.get(), bytes.length());
  } catch (const std::exception& exc) {
    throw KVSException(KVSErrorType::STORAGE_WRITE_FAILED);
  }
  fileSize += bytes.length();
  return prevFileSize;
}

} // namespace kvs::storage
