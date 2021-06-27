#include "BloomFilter.h"
#include "doctest.h"
#include <random>
#include <string>
#include <unordered_set>

namespace std {

// required for std::unord_set<Key>
template <>
struct hash<kvs::Key> {
    std::size_t operator()(const kvs::Key& key) const noexcept { return kvs::hashKey(key, 0); }
};

} // namespace std

namespace kvs::bloom_filter::test {

TEST_CASE("test BloomFilter") {
    SUBCASE("simple") {
        BloomFilter filter;
        kvs::Key key1(5);
        CHECK(filter.checkExist(key1) == false);
        filter.add(key1);
        CHECK(filter.checkExist(key1) == true);
        filter.add(key1);
        filter.add(key1);
        filter.add(key1);
        CHECK(filter.checkExist(key1) == true);
        kvs::Key key2(555);
        CHECK(filter.checkExist(key2) == false);
        filter.add(key2);
        CHECK(filter.checkExist(key2) == true);
    }
    SUBCASE("stress") {
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<key_t> distr;
        const size_t OPS = 5000; // ! see constants

        BloomFilter filter;
        std::unordered_set<Key> set(OPS);
        size_t totalChecks = 0, filterMisses = 0;
        for (size_t i = 0; i < OPS; i++) {
            key_t k = rand();
            kvs::Key key(k);
            if (rand() & 1) {
                // add
                filter.add(key);
                set.insert(key);
            } else {
                // check
                totalChecks++;
                if (set.find(key) == set.end()) {
                    if (filter.checkExist(key) == true)
                        filterMisses++;
                } else {
                    // CHECK will spam the terminal
                    REQUIRE(filter.checkExist(key) == true);
                }
            }
        }
        MESSAGE(std::string("filter missed ") + std::to_string(filterMisses) + " out of " +
                std::to_string(totalChecks) + " checks");
    }
}

} // namespace kvs::bloom_filter::test