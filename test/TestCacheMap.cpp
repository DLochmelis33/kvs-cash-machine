#include "CacheMap.h"
#include "doctest.h"
#include <cstring>
#include <random>
#include <unordered_map>

namespace kvs_test::cache_map {

using namespace kvs::utils;
using namespace kvs::cache_map;

Key generateKey(size_t value) {
  ByteArray byteArray(KEY_SIZE);
  std::memcpy(byteArray.get(), reinterpret_cast<char*>(&value), sizeof(size_t));
  return Key{byteArray};
}

TEST_CASE("test CacheMap") {

  const Ptr p1(1 * VALUE_SIZE, true), p2(2 * VALUE_SIZE, true),
      p3(3 * VALUE_SIZE, true);
  const Entry e1(generateKey(1), p1), e2(generateKey(2), p2),
      e3(generateKey(3), p3);
  const Entry e4(generateKey(4), p1), e5(generateKey(5), p2),
      e6(generateKey(6), p3);

  SUBCASE("test simple") {
    CacheMap map(5);
    std::optional<Entry> opt;
    CHECK(map.get(e1.key) == EMPTY_PTR);
    CHECK(map.get(e2.key) == EMPTY_PTR);
    CHECK(map.get(e3.key) == EMPTY_PTR);

    opt = map.putOrDisplace(e1);
    CHECK(map.get(e1.key) == p1);
    CHECK(map.get(e2.key) == EMPTY_PTR);
    CHECK(map.get(e3.key) == EMPTY_PTR);
    CHECK(!opt.has_value());

    opt = map.putOrDisplace(e2);
    CHECK(map.get(e1.key) == p1);
    CHECK(map.get(e2.key) == p2);
    CHECK(map.get(e3.key) == EMPTY_PTR);
    CHECK(!opt.has_value());

    opt = map.putOrDisplace(e3);
    CHECK(map.get(e1.key) == p1);
    CHECK(map.get(e2.key) == p2);
    CHECK(map.get(e3.key) == p3);
    CHECK(!opt.has_value());

    opt = map.putOrDisplace(Entry(generateKey(1), Ptr(5)));
    CHECK(map.get(e1.key) == Ptr(5));
    CHECK(map.get(e2.key) == p2);
    CHECK(map.get(e3.key) == p3);
    CHECK(!opt.has_value());

    opt = map.putOrDisplace(Entry(generateKey(2), Ptr(5)));
    opt = map.putOrDisplace(Entry(generateKey(3), Ptr(5)));
    CHECK(map.get(e1.key) == Ptr(5));
    CHECK(map.get(e2.key) == Ptr(5));
    CHECK(map.get(e3.key) == Ptr(5));
    CHECK(!opt.has_value());
  }

  SUBCASE("test displace") {
    CacheMap map(5);
    std::optional<Entry> opt;
    CHECK((opt = map.putOrDisplace(e1), !opt.has_value()));
    CHECK((opt = map.putOrDisplace(e2), !opt.has_value()));
    CHECK((opt = map.putOrDisplace(e3), !opt.has_value()));
    // displacement lags one op behind
    CHECK((opt = map.putOrDisplace(e4), !opt.has_value()));
    CHECK((opt = map.putOrDisplace(e5), opt.has_value()));
    CHECK((opt = map.putOrDisplace(e6), opt.has_value()));

    for (size_t i = 0; i < 100; i++) map.putOrDisplace(e1); // probabilistic

    CHECK(map.get(e2.key) == EMPTY_PTR);
  }

  SUBCASE("test stress") {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> keyDistr;
    srand(time(NULL));

    std::unordered_map<uint64_t, ptr_t> realMap;
    CacheMap map(20000);

    for (size_t ops = 0; ops < 50000; ops++) {
      uint64_t k = keyDistr(gen);
      ptr_t p = static_cast<unsigned char>(rand() & 0b01111111);
      if (p == 0b01111111)
        p--;
      Key key = generateKey(k);
      Ptr ptr(rand(), rand());
      Entry entry(key, ptr);

      if (rand() & 1) {
        // put
        map.putOrDisplace(entry);
        realMap[k] = p;
      } else {
        // get
        Ptr got = map.get(key);
        ptr_t real =
            (realMap.find(k) == realMap.end() ? Ptr::EMPTY_PTR_V : realMap[k]);
        // CHECK will spam the terminal
        REQUIRE(got.get() == real);
      }
    }
  }
}

} // namespace kvs_test::cache_map