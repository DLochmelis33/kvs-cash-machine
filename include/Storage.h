#pragma once

#include "ByteArray.h"

#include <fstream>
#include <string>

namespace kvs::storage {

/**
 * @brief Read the entire file.
 *
 */
utils::ByteArray readFile(std::string filename); // TODO docs: empty file is ok

/**
 * @brief Write the entire file. Create file if it's not created.
 *
 */
void writeFile(std::string filename, utils::ByteArray bytes);
// TODO docs: file is truncated if it exists, zero-length bytes is ok

/**
 * @brief An abstraction for safely opening, reading, writing and closing files on disk.
 * 
 * If any operation fails, throws a new KVSException.
 *
 */
class Storage final {
public:
  explicit Storage(
      std::string filename); // TODO docs: use only for existent filename

  /**
    * @brief Apply all changes to the file contents (if any) and close the file. All further operations with this Storage object are disallowed.
    * 
    */
  void close();

  /**
     * @brief Read a part of file. // TODO docs: offset from the beggining; zero length is ok
     *
     */
  utils::ByteArray read(size_t offset, size_t length);

  /**
     * @brief Write a part of file. // TODO docs: offset from the beggining; zero length is ok; if offset + bytes.length() > eof append
     *
     */
  void write(size_t offset, utils::ByteArray bytes);

  /**
    * @brief Append to end of file.
    * 
    * @return The size of file in bytes before appending.
    */
  size_t append(utils::ByteArray bytes); // TODO docs: zero length is ok

private:
  std::fstream file;
  size_t fileSize;
};

} // namespace kvs::storage
