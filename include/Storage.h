#pragma once

#include "KeyValueTypes.h"
#include <fstream>

namespace kvs::storage {

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
     * @brief Read the entire file.
     *
     */
  utils::ByteArray readFile() const;

  /**
     * @brief Read a part of file.
     *
     */
  utils::ByteArray read(size_t offset, size_t length) const;

  /**
     * @brief Write the entire file.
     *
     */
  void writeFile(utils::ByteArray bytes) const;

  /**
     * @brief Write a part of file.
     *
     */
  void write(size_t offset, utils::ByteArray bytes) const;

  /**
    * @brief Append to end of file.
    * 
    * @return The size of file in bytes before appending.
    */
  size_t append(utils::ByteArray bytes) const;

private:
  std::fstream file;
};

} // namespace kvs::storage
