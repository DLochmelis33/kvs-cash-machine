#include "Storage.h"
#include "doctest.h"
#include <filesystem>
#include <iostream>

using namespace kvs::storage;
using kvs::utils::ByteArray;

namespace test_kvs::storage {

const std::string testDirectoryPath = "../.test-data/test-storage/";
const std::string filePath = testDirectoryPath + "file.test";
const std::string emptyFilePath = testDirectoryPath + "empty-file.test";
constexpr uint32_t maxContentInteger = 1e6;
constexpr size_t contentFileSize = (maxContentInteger + 1) * sizeof(uint32_t);

void setUpTestDirectory() {
  std::filesystem::create_directories(testDirectoryPath);

  std::ofstream file(filePath,
                     std::ios::out | std::ios::binary | std::ios::trunc);
  file.exceptions(std::ofstream::failbit | std::ofstream::badbit);
  std::string content;
  for (uint32_t i = 0; i <= maxContentInteger; ++i) {
    file.write(reinterpret_cast<char*>(&i), sizeof(uint32_t));
  }
  file.close();

  std::ofstream emptyFile(emptyFilePath);
  emptyFile.exceptions(std::ofstream::failbit | std::ofstream::badbit);
  emptyFile.close();
}

void clearTestDirectory() { std::filesystem::remove_all(testDirectoryPath); }

ByteArray serializeIntToByteArray(uint32_t value) {
  ByteArray bytes(sizeof(uint32_t));
  char* valueBytes = reinterpret_cast<char*>(&value);
  for (size_t i = 0; i < sizeof(uint32_t); ++i) {
    bytes.get()[i] = valueBytes[i];
  }
  return bytes;
}

TEST_CASE("test readFile and writeFile") {
  setUpTestDirectory();

  SUBCASE("test readFile") {

    SUBCASE("read emptyFile") {
      ByteArray content = readFile(emptyFilePath);
      CHECK(content.length() == 0);
    }

    SUBCASE("read file with content") {
      ByteArray content = readFile(filePath);
      REQUIRE(content.length() == contentFileSize);
      for (uint32_t i = 0; i <= maxContentInteger; ++i) {
        CHECK(*reinterpret_cast<uint32_t*>(content.get() +
                                           i * sizeof(uint32_t)) == i);
      }
    }
  }

  SUBCASE("test writeFile") {

    SUBCASE("zero-length bytes") {
      ByteArray zeroBytes(0);
      std::string fileToWritePath;

      SUBCASE("write to not-existent file") {
        fileToWritePath = testDirectoryPath + "fileToCreate";
      }
      SUBCASE("truncate file with content") { fileToWritePath = filePath; }

      writeFile(fileToWritePath, zeroBytes);
      REQUIRE(std::filesystem::exists(fileToWritePath));
      CHECK(std::filesystem::file_size(fileToWritePath) == 0);
    }

    SUBCASE("nonzero-length bytes") {
      size_t contentSize = 100;
      ByteArray content(contentSize);
      for (unsigned char c = 0; c < contentSize; ++c) {
        content.get()[c] = c;
      }
      std::string fileToWritePath;

      SUBCASE("write to not-existent file") {
        fileToWritePath = testDirectoryPath + "fileToCreate";
      }
      SUBCASE("rewrite file with content") { fileToWritePath = filePath; }

      writeFile(fileToWritePath, content);
      REQUIRE(std::filesystem::file_size(fileToWritePath) == contentSize);

      std::ifstream file(fileToWritePath, std::ios::in | std::ios::binary);
      file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
      for (unsigned char c = 0; c < contentSize; ++c) {
        unsigned char readChar;
        file.read(reinterpret_cast<char*>(&readChar), sizeof(unsigned char));
        CHECK(readChar == c);
      }
      CHECK(file.peek() == EOF);
      file.close();
    }
  }

  clearTestDirectory();
}

TEST_CASE("test Storage") {
  setUpTestDirectory();

  SUBCASE("test create and close") {
    Storage storage(filePath);
    storage.close();
  }

  SUBCASE("test read") {

    SUBCASE("read zero bytes from empty file") {
      Storage storage(emptyFilePath);
      ByteArray content = storage.read(0, 0);
      storage.close();
      CHECK(content.length() == 0);
    }

    SUBCASE("read file with content") {
      Storage storage(filePath);
      for (uint32_t i = 0; i <= maxContentInteger; i += 2) {
        ByteArray content =
            storage.read(i * sizeof(uint32_t), sizeof(uint32_t));
        REQUIRE(content.length() == sizeof(uint32_t));
        CHECK(*reinterpret_cast<uint32_t*>(content.get()) == i);
      }
      storage.close();
    }
  }

  SUBCASE("test write") {

    SUBCASE("write zero bytes") {
      Storage storage(emptyFilePath);
      storage.write(0, ByteArray(0));
      storage.close();
      CHECK(std::filesystem::file_size(emptyFilePath) == 0);
    }

    SUBCASE("rewrite some bytes") {
      Storage storage(filePath);
      for (uint32_t i = 0; i <= maxContentInteger; i += 2) {
        storage.write(i * sizeof(uint32_t), serializeIntToByteArray(i * 2));
      }
      storage.close();

      ByteArray content = readFile(filePath);
      REQUIRE(content.length() == contentFileSize);
      for (uint32_t i = 0; i <= maxContentInteger; ++i) {
        if (i % 2 == 0) {
          CHECK(*reinterpret_cast<uint32_t*>(content.get() +
                                             i * sizeof(uint32_t)) == i * 2);
        } else {
          CHECK(*reinterpret_cast<uint32_t*>(content.get() +
                                             i * sizeof(uint32_t)) == i);
        }
      }
    }

    SUBCASE("write further end of file") {
      uint32_t valueToAppend = 65;
      Storage storage(filePath);
      storage.write(contentFileSize, serializeIntToByteArray(valueToAppend));
      storage.close();

      ByteArray content = readFile(filePath);
      REQUIRE(content.length() == contentFileSize + sizeof(uint32_t));
      for (uint32_t i = 0; i <= maxContentInteger; ++i) {
        CHECK(*reinterpret_cast<uint32_t*>(content.get() +
                                           i * sizeof(uint32_t)) == i);
      }
      CHECK(*reinterpret_cast<uint32_t*>(content.get() + contentFileSize) ==
            valueToAppend);
    }
  }

  SUBCASE("test append") {

    SUBCASE("append zero bytes") {
      Storage storage(emptyFilePath);
      size_t fileSize = storage.append(ByteArray(0));
      storage.close();
      CHECK(fileSize == 0);
      CHECK(std::filesystem::file_size(emptyFilePath) == 0);
    }

    SUBCASE("append some bytes") {
      Storage storage(filePath);
      uint32_t appendValuesNumber = 1000;
      for (uint32_t i = maxContentInteger + 1;
           i <= maxContentInteger + appendValuesNumber; ++i) {
        size_t fileSize = storage.append(serializeIntToByteArray(i));
        REQUIRE(fileSize == contentFileSize +
                                (i - maxContentInteger - 1) * sizeof(uint32_t));
      }
      storage.close();

      ByteArray content = readFile(filePath);
      REQUIRE(content.length() ==
              contentFileSize + appendValuesNumber * sizeof(uint32_t));
      for (uint32_t i = 0; i <= maxContentInteger + appendValuesNumber; ++i) {
        CHECK(*reinterpret_cast<uint32_t*>(content.get() +
                                           i * sizeof(uint32_t)) == i);
      }
    }
  }

  SUBCASE("test complex") {
    Storage storage(filePath);
    for (uint32_t i = 0; i <= maxContentInteger; i += 2) {
      ByteArray content = storage.read(i * sizeof(uint32_t), sizeof(uint32_t));
      REQUIRE(content.length() == sizeof(uint32_t));
      CHECK(*reinterpret_cast<uint32_t*>(content.get()) == i);
    }

    uint32_t appendValuesNumber = 1000;
    for (uint32_t i = maxContentInteger + 1;
         i <= maxContentInteger + appendValuesNumber; ++i) {
      size_t fileSize = storage.append(serializeIntToByteArray(i));
      REQUIRE(fileSize ==
              contentFileSize + (i - maxContentInteger - 1) * sizeof(uint32_t));
    }
    for (uint32_t i = 0; i <= maxContentInteger + appendValuesNumber; i += 2) {
      storage.write(i * sizeof(uint32_t), serializeIntToByteArray(i * 2));
    }

    for (uint32_t i = 0; i <= maxContentInteger + appendValuesNumber; ++i) {
      ByteArray content = storage.read(i * sizeof(uint32_t), sizeof(uint32_t));
      REQUIRE(content.length() == sizeof(uint32_t));
      uint32_t readValue = *reinterpret_cast<uint32_t*>(content.get());
      if (i % 2 == 0) {
        CHECK(readValue == i * 2);
      } else {
        CHECK(readValue == i);
      }
    }
    storage.close();
  }

  clearTestDirectory();
}

} // namespace test_kvs::storage
