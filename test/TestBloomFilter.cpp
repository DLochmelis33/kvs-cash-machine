#include "BloomFilter.h"
#include "doctest.h"

#include <cstring>
#include <random>
#include <string>
#include <unordered_set>

using namespace kvs::bloom_filter;

namespace std {

// required for std::unordered_set<Key>
template <>
struct hash<kvs::utils::Key> {
  std::size_t operator()(const kvs::utils::Key& key) const noexcept {
    return kvs::utils::hashKey(key, 0);
  }
};

} // namespace std

namespace test_kvs::bloom_filter {

Key generateKey(size_t value) {
  ByteArray byteArray{KEY_SIZE};
  std::memcpy(byteArray.get(), reinterpret_cast<char*>(&value), sizeof(size_t));
  return Key{byteArray};
}

TEST_CASE("test BloomFilter") {
  SUBCASE("test simple") {
    BloomFilter filter;
    Key key1 = generateKey(5);
    CHECK(filter.checkExist(key1) == false);
    filter.add(key1);
    CHECK(filter.checkExist(key1) == true);
    filter.add(key1);
    filter.add(key1);
    filter.add(key1);
    CHECK(filter.checkExist(key1) == true);
    Key key2 = generateKey(555);
    CHECK(filter.checkExist(key2) == false);
    filter.add(key2);
    CHECK(filter.checkExist(key2) == true);
  }
  SUBCASE("test stress") {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<__uint128_t> distr;
    const size_t OPS = 5000; // ! see constants

    BloomFilter filter;
    std::unordered_set<Key> set(OPS);
    size_t totalChecks = 0, filterMisses = 0;
    for (size_t i = 0; i < OPS; i++) {
      Key key = generateKey(distr(gen));
      if (rand() & 1) {
        // add
        filter.add(key);
        set.insert(key);
      } else {
        // check
        totalChecks++;
        if (set.find(key) == set.end()) {
          if (filter.checkExist(key))
            filterMisses++;
        } else {
          // CHECK will spam the terminal
          REQUIRE(filter.checkExist(key));
        }
      }
    }
    MESSAGE(std::string("filter missed ") + std::to_string(filterMisses) +
            " out of " + std::to_string(totalChecks) + " checks");
  }
}

} // namespace test_kvs::bloom_filter
