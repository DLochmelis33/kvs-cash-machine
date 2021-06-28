#include "KVS.h"
#include "doctest.h"
#include <cstring>

using namespace kvs;
using namespace kvs::utils;

namespace test_kvs::kvs {

Key generateKey(size_t value) {
  ByteArray byteArray(KEY_SIZE);
  std::memcpy(byteArray.get(), reinterpret_cast<char*>(&value), sizeof(size_t));
  return Key{byteArray};
}

Value generateValue(int seed) {
    srand(seed);
    
}

const Ptr p1(1 * VALUE_SIZE, true), p2(2 * VALUE_SIZE, true),
    p3(3 * VALUE_SIZE, true);
const Entry e1(generateKey(1), p1), e2(generateKey(2), p2),
    e3(generateKey(3), p3);
const Entry e4(generateKey(4), p1), e5(generateKey(5), p2),
    e6(generateKey(6), p3);


TEST_CASE("test KVS") {

  SUBCASE("test simple") {
    KVS kvs;
    kvs.add()
  }
}

} // namespace test_kvs::kvs