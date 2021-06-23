#include "ByteArray.h"
#include "doctest.h"

using kvs::utils::ByteArray;

TEST_CASE("test ByteArray") {
  SUBCASE("test simple") {
    size_t length = 100;
    ByteArray byteArray(length);
    REQUIRE(byteArray.length() == length);

    byteArray.charPtrData()[0] = 'a';
    CHECK(byteArray.charPtrData()[0] == 'a');
  }
  SUBCASE("test write and read") {
    for (size_t len = 1; len < 100; ++len) {
      ByteArray byteArray(len);
      REQUIRE(byteArray.length() == len);

      for (size_t i = 0; i < len; ++i) {
        byteArray.charPtrData()[i] = static_cast<char>(i);
      }
      for (size_t i = 0; i < len; ++i) {
        CHECK(byteArray.charPtrData()[i] == static_cast<char>(i));
      }
    }
  }
}
