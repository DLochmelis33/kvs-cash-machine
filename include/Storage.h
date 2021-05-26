#pragma once

#include "KeyValueTypes.h"
#include <exception>
#include <string>

/**
 * @brief An abstraction for safely opening, reading, writing and closing files on disk.
 *
 */
class Storage {
public:
  struct ByteArray {
    const char* data;
    size_t length;
  };

  explicit Storage(std::string filename);

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
  void writeFile(ByteArray bytes) const; // TODO: exceptions

  /**
     * @brief Write a part of file.
     *
     * @param offset
     * @param length
     */
  void write(size_t offset, ByteArray bytes) const; // TODO: exceptions

   /**
    * @brief Append to end of file.
    * 
    * @param bytes 
    * @return size_t the size of file in bytes before appending
    */
  size_t append(ByteArray bytes) const;

private:
  std::string filename;
};
