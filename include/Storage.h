#pragma once

#include "KeyValueTypes.h"
#include <fstream>

/**
 * @brief An abstraction for safely opening, reading, writing and closing files on disk.
 *
 */
class Storage final {
public:
  explicit Storage(std::string filename);

  void close();

  /**
     * @brief Read the entire file.
     *
     * @return char*
     */
  ByteArray readFile() const;

  /**
     * @brief Read a part of file.
     *
     * @param offset
     * @param length
     * @return ByteArray
     */
  ByteArray read(size_t offset, size_t length) const;

  /**
     * @brief Write the entire file.
     *
     * @return char*
     */
  void writeFile(ByteArray bytes) const;

  /**
     * @brief Write a part of file.
     *
     * @param offset
     * @param length
     */
  void write(size_t offset, ByteArray bytes) const;

  /**
    * @brief Append to end of file.
    * 
    * @param bytes 
    * @return size_t the size of file in bytes before appending
    */
  size_t append(ByteArray bytes) const;

private:
  std::fstream file;
};
