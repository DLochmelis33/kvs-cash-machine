#pragma once

#include "KeyValueTypes.h"
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
    ByteArray read(size_t offset, size_t length);

    /**
     * @brief Write the entire file.
     *
     * @return char*
     */
    void writeFile(const char* data) const; // TODO: exceptions

    /**
     * @brief Write a part of file.
     *
     * @param offset
     * @param length
     */
    void write(size_t offset, size_t length, const char* data); // TODO: exceptions

  private:
    // TODO
};