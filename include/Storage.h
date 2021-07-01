#pragma once

#include "ByteArray.h"

#include <fstream>
#include <string>

namespace kvs::storage {

using kvs::utils::ByteArray;

/**
 * @brief Read the entire file. 
 * 
 * If the file is empty, returns a ByteArray of zero length.
 *
 */
ByteArray readFile(std::string filename);

/**
 * @brief Overwrite the entire file - that is, truncate, write and close it. Create file if it doesn't exist.
 * 
 * Supports zero-length ByteArray.
 *
 */
void writeFile(std::string filename, ByteArray bytes);

/**
 * @brief An abstraction for safely opening, reading, writing and closing files on disk.
 * 
 * If any operation fails, throws a new KVSException.
 *
 */
class Storage final {
public:
  /**
   * @brief Construct a new Storage from the specified file. Fails if the file doesn't exist.
   * 
   * @throws KVSException if the file doesn't exist.
   */
  explicit Storage(std::string filename);

  /**
    * @brief Apply all changes to the file contents (if any) and close the file. All further operations with this Storage object are disallowed.
    * 
    */
  void close();

  /**
     * @brief Read a part of file. 
     * 
     * @param offset The offset from the beggining of the file.
     * @param length The length of the part to read. Can be 0.
     *
     */
  ByteArray read(size_t offset, size_t length);

  /**
     * @brief Write a part of file. If the specified part exceeds the end of file, extra data is appended.
     *
     * @param offset The offset from the beggining of the file.
     * @param length The length of the part to read. Can be 0.
     * 
     */
  void write(size_t offset, ByteArray bytes);

  /**
    * @brief Append to end of file.
    * 
    * Supports zero-length ByteArray.
    * 
    * @return The size of file in bytes before appending.
    */
  size_t append(ByteArray bytes);

private:
  std::fstream file;
  size_t fileSize;
};

} // namespace kvs::storage
