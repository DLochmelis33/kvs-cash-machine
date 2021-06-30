#include "KVS.h"
#include "doctest.h"
#include <filesystem>
#include <random>
#include <unordered_map>

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

const std::string testDirectoryPath = "../.test-data/test-kvs/";

void setUpTestDirectory() {
  std::filesystem::create_directories(testDirectoryPath);
  Shard::storageDirectoryPath = testDirectoryPath;
}

void clearTestDirectory() { std::filesystem::remove_all(testDirectoryPath); }

std::random_device rd;
std::mt19937_64 gen(rd());
std::uniform_int_distribution<char> charDistr(std::numeric_limits<char>::min(),
                                              std::numeric_limits<char>::max());
constexpr uint32_t maxOpDistrRange = 1e6;
std::uniform_int_distribution<uint32_t> opDistr(0, maxOpDistrRange);

constexpr double removeOperationsRate =
    0.2; // between write and remove operations
constexpr double readOperationsRate = 0.5;

uint8_t generateRandomOperationCode() {
  uint32_t num = opDistr(gen);
  uint32_t maxReadNum = maxOpDistrRange * readOperationsRate;
  if (num <= maxReadNum) {
    return 0;
  }
  uint32_t writeRemoveNum = num - maxReadNum;
  if (writeRemoveNum < writeRemoveNum * removeOperationsRate) {
    return 1;
  }
  return 2;
}

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

Key generateNewRandomKey(std::unordered_map<Key, Value>& mapKVS) {
  Key key = generateRandomKey();
  while (mapKVS.find(key) != mapKVS.end()) {
    Key key = generateRandomKey();
  }
  return key;
}

TEST_CASE("test KVS") {

  setUpTestDirectory();

  SUBCASE("test simple add and get") {
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

  SUBCASE("stress test") {
    size_t setupElementsSize = 1e4;
    size_t operationsNumber = 9e4;

    std::unordered_map<Key, Value> mapKVS;
    KVS kvs{};
    for (size_t i = 0; i < setupElementsSize; ++i) {
      Key key = generateNewRandomKey(mapKVS);
      Value value = generateRandomValue();
      kvs.add(key, value);
      mapKVS[key] = value;
      REQUIRE(kvs.get(key).value() == value);
    }

    for (size_t i = 0; i < operationsNumber; ++i) {
      Key key = generateRandomKey();
      uint8_t operationCode = generateRandomOperationCode();

      switch (operationCode) {
      case 0: {
        std::optional<Value> optValue = kvs.get(key);
        const auto& it = mapKVS.find(key);
        if (it == mapKVS.end()) {
          REQUIRE_FALSE(optValue.has_value());
        } else {
          REQUIRE((*it).second == optValue.value());
        }
        break;
      }
      case 1: {
        kvs.remove(key);
        mapKVS.erase(key);
        break;
      }
      case 2: {
        Value value = generateRandomValue();
        kvs.add(key, value);
        mapKVS[key] = value;
        break;
      }
      }
    }
  }

  clearTestDirectory();
}

} // namespace test_kvs::kvs
