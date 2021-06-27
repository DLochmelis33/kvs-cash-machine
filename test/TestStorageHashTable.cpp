#include "StorageHashTable.h"
#include "doctest.h"

#include <bitset>
#include <cstring>
#include <iostream>
#include <random>
#include <sstream>
#include <unordered_map>

using namespace kvs::utils;
using namespace kvs::storage_hash_table;

namespace test_kvs::storage_hash_table {

template <typename T>
bool contains(const std::vector<T>& holder, const T& value) {
  for (const T& x : holder) {
    if (x == value)
      return true;
  }
  return false;
}

template <typename T>
bool containsAll(const std::vector<T>& holder, const std::vector<T>& values) {
  for (const T& value : values) {
    if (!contains(holder, value))
      return false;
  }
  return true;
}

std::string toBin(Key key) {
  std::stringstream ss;
  for (size_t i = KEY_SIZE - 1; i < KEY_SIZE; i--) {
    std::bitset<8> rep(key.getBytes().get()[i]);
    ss << rep;
  }
  return ss.str();
}

std::string toBin(ptr_t p) {
  std::stringstream ss;
  ss << std::bitset<8>(p);
  return ss.str();
}

std::string print(const std::vector<Entry>& vec) {
  std::stringstream ss;
  ss << "{ ";
  for (Entry e : vec) {
    ss << "[";
    ss << toBin(e.key);
    ss << ", " << toBin(e.ptr.getRaw()) << "],\n";
  }
  ss << "}\n";
  return ss.str();
}

Key generateKey(size_t value) {
  ByteArray byteArray(KEY_SIZE);
  std::memcpy(byteArray.get(), reinterpret_cast<char*>(&value), sizeof(size_t));
  return Key{byteArray};
}

TEST_CASE("test StorageHashTable") {

  const Ptr p1(1 * VALUE_SIZE, true), p2(2 * VALUE_SIZE, true),
      p3(3 * VALUE_SIZE, true);
  const Entry e1(generateKey(1), p1), e2(generateKey(2), p2),
      e3(generateKey(3), p3);
  const Entry e4(generateKey(4), p1), e5(generateKey(5), p2),
      e6(generateKey(6), p3);

  SUBCASE("operations") {

    SUBCASE("test simple") {
      StorageHashTable table(5);
      CHECK(table.get(e1.key) == EMPTY_PTR);
      CHECK(table.get(e2.key) == EMPTY_PTR);
      CHECK(table.get(e3.key) == EMPTY_PTR);

      table.put(e1);
      CHECK(table.get(e1.key) == p1);
      CHECK(table.get(e2.key) == EMPTY_PTR);
      CHECK(table.get(e3.key) == EMPTY_PTR);

      table.put(e2);
      CHECK(table.get(e1.key) == p1);
      CHECK(table.get(e2.key) == p2);
      CHECK(table.get(e3.key) == EMPTY_PTR);

      table.put(e3);
      CHECK(table.get(e1.key) == p1);
      CHECK(table.get(e2.key) == p2);
      CHECK(table.get(e3.key) == p3);

      table.put(Entry(generateKey(1), Ptr(5)));
      CHECK(table.get(e1.key) == Ptr(5));
      CHECK(table.get(e2.key) == p2);
      CHECK(table.get(e3.key) == p3);

      table.put(Entry(generateKey(2), Ptr(5)));
      table.put(Entry(generateKey(3), Ptr(5)));
      CHECK(table.get(e1.key) == Ptr(5));
      CHECK(table.get(e2.key) == Ptr(5));
      CHECK(table.get(e3.key) == Ptr(5));
    }

    SUBCASE("with added entries") {
      StorageHashTable table(5);
      table.put(e1);
      table.put(e2);
      table.put(e3);
      SUBCASE("getEntries()") {
        CHECK(containsAll(table.getEntries(), {e1, e2, e3}));
      }
      SUBCASE("expand()") {
        // MESSAGE(std::string("hello ") + std::to_string(table.getSize()));

        table.put(e4); // 4 * 1.5 ==> expands

        CHECK(table.get(e1.key) == p1);
        CHECK(table.get(e2.key) == p2);
        CHECK(table.get(e3.key) == p3);
        CHECK(table.get(e4.key) == p1);

        table.put(e5);
        table.put(e6); // if not expanded, this won't fit => fails

        CHECK(table.get(e1.key) == p1);
        CHECK(table.get(e2.key) == p2);
        CHECK(table.get(e3.key) == p3);
        CHECK(table.get(e4.key) == p1);
        CHECK(table.get(e5.key) == p2);
        CHECK(table.get(e6.key) == p3);
      }
    }

    SUBCASE("stress") {
      std::random_device rd;
      std::mt19937_64 gen(rd());
      std::uniform_int_distribution<uint64_t> keyDistr;
      srand(time(NULL));

      static_assert(TABLE_MAX_SIZE >= 20000);
      std::unordered_map<uint64_t, ptr_t> realMap;
      StorageHashTable table(20000);

      for (size_t ops = 0; ops < 10000; ops++) {
        uint64_t k = keyDistr(gen);
        ptr_t p = static_cast<unsigned char>(rand() & 0b01111111);
        if (p == 0b01111111)
          p--;
        Key key = generateKey(k);
        Ptr ptr(rand(), rand());
        Entry entry(key, ptr);

        if (rand() & 1) {
          // put
          table.put(entry);
          realMap[k] = p;
        } else {
          // get
          Ptr got = table.get(key);
          ptr_t real = (realMap.find(k) == realMap.end() ? Ptr::EMPTY_PTR_V
                                                         : realMap[k]);
          // CHECK will spam the terminal
          REQUIRE(got.get() == real);
        }
      }
    }

    SUBCASE("storing") {
      StorageHashTable table(5);

      SUBCASE("empty table") {
        ByteArray serialized = table.serializeToByteArray();
        StorageHashTable built(serialized);
        CHECK(built.getEntries().size() == 0);
      }

      SUBCASE("nonempty table") {
        table.put(e1);
        table.put(e2);
        table.put(e3);

        ByteArray serialized = table.serializeToByteArray();
        StorageHashTable built(serialized);

        CHECK(containsAll(built.getEntries(), {e1, e2, e3}));
      }

      SUBCASE("resized table") {
        table.put(e1);
        table.put(e2);
        table.put(e3);
        table.put(e4);
        table.put(e5);
        table.put(e6);

        ByteArray serialized = table.serializeToByteArray();
        StorageHashTable built(serialized);
        CHECK(containsAll(built.getEntries(), {e1, e2, e3, e4, e5, e6}));
      }
    }
  }
}

} // namespace test_kvs::storage_hash_table
