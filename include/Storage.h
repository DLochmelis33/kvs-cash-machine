#pragma once

#include "ByteArray.h"

#include <fstream>
#include <string>

namespace kvs::storage {

/**
 * @brief Read the entire file.
 *
 */
utils::ByteArray readFile(std::string filename);

/**
 * @brief Write the entire file.
 *
 */
void writeFile(std::string filename, utils::ByteArray bytes);

/**
 * @brief An abstraction for safely opening, reading, writing and closing files on disk.
 * 
 * If any operation fails, throws a new KVSException.
 *
 */
class Storage final {
public:
  explicit Storage(std::string filename);

  /**
    * @brief Apply all changes to the file contents (if any) and close the file. All further operations with this Storage object are disallowed.
    * 
    */
  void close();

  /**
     * @brief Read a part of file. // TODO: offset from the beggining
     *
     */
  utils::ByteArray read(size_t offset, size_t length);

  /**
     * @brief Write a part of file. // TODO: offset from the beggining
     *
     */
  void write(size_t offset, utils::ByteArray bytes);

  /**
    * @brief Append to end of file.
    * 
    * @return The size of file in bytes before appending.
    */
  size_t append(utils::ByteArray bytes);

private:
  std::fstream file;
  size_t fileSize;
};

} // namespace kvs::storage
