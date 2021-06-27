#include "ByteArray.h"
#include "doctest.h"

using kvs::utils::ByteArray;

TEST_CASE("test ByteArray") {

  SUBCASE("test simple") {
    size_t length = 100;
    ByteArray byteArray(length);
    REQUIRE(byteArray.length() == length);

    byteArray.get()[0] = 'a';
    CHECK(byteArray.get()[0] == 'a');
  }

  SUBCASE("test write and read") {
    for (size_t len = 1; len < 100; ++len) {
      ByteArray byteArray(len);
      REQUIRE(byteArray.length() == len);

      for (size_t i = 0; i < len; ++i) {
        byteArray.get()[i] = static_cast<char>(i);
      }
      for (size_t i = 0; i < len; ++i) {
        CHECK(byteArray.get()[i] == static_cast<char>(i));
      }
    }
  }

  SUBCASE("test zero-length") {
    ByteArray byteArray(0);
    REQUIRE(byteArray.length() == 0);
    CHECK_NOTHROW(byteArray.get());
  }
}
