#include "KVS.h"
#include "doctest.h"
#include <cstring>
#include <random>
#include <sstream>
#include <unordered_set>

namespace std {

// required for std::unordered_set<Key>
template <>
struct hash<kvs::utils::Key> {
  std::size_t operator()(const kvs::utils::Key& key) const noexcept {
    return kvs::utils::hashKey(key, 0);
  }
};

} // namespace std

using namespace kvs;
using namespace kvs::utils;

namespace test_kvs::kvs {

std::random_device rd;
std::mt19937_64 gen(rd());
std::uniform_int_distribution<char> charDistr(std::numeric_limits<char>::min(),
                                              std::numeric_limits<char>::max());

ByteArray generateRandomByteArray(size_t size) {
  ByteArray bytes(size);
  char* charptr = bytes.get();
  for (size_t i = 0; i < size; ++i) {
    charptr[i] = charDistr(gen);
  }
  return bytes;
}

Key generateRandomKey() { return Key{generateRandomByteArray(KEY_SIZE)}; }

Value generateRandomValue() {
  return Value{generateRandomByteArray(VALUE_SIZE)};
}

Key generateNewRandomKey(std::unordered_set<Key>& keys) {
  Key key = generateRandomKey();
  while (keys.find(key) != keys.end()) {
    Key key = generateRandomKey();
  }
  keys.insert(key);
  return key;
}

std::string byteArrayToStr(ByteArray ba) {
  std::stringstream ss;
  for (size_t i = 0; i < ba.length(); i++) {
    ss << (int) ba.get()[i] << ' ';
  }
  return ss.str();
}

TEST_CASE("test KVS") {

  SUBCASE("test simple") {
    KVS kvs;

    Key k1 = generateRandomKey();
    Value v1 = generateRandomValue();
    kvs.add(k1, v1);
    std::optional<Value> optVal = kvs.get(k1);
    CHECK(optVal.has_value());
    CHECK(optVal.value() == v1);

    Key k2 = generateRandomKey();
    Value v2 = generateRandomValue();
    kvs.add(k2, v2);
    optVal = kvs.get(k2);
    CHECK(optVal.has_value());
    CHECK(optVal.value() == v2);
  }

  SUBCASE("test simple remove") {
    KVS kvs;

    Key k1 = generateRandomKey();
    Value v1 = generateRandomValue();
    kvs.add(k1, v1);

    Key k2 = generateRandomKey();
    Value v2 = generateRandomValue();
    kvs.add(k2, v2);

    std::optional<Value> optVal = kvs.get(k1);
    CHECK(optVal.has_value());
    CHECK(optVal.value() == v1);

    kvs.remove(k1);
    optVal = kvs.get(k1);
    CHECK(!optVal.has_value());
  }
}

} // namespace test_kvs::kvs