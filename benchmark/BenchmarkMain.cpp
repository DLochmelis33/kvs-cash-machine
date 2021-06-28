#include "KVS.h"

#include <filesystem>
#include <iostream>
#include <limits>
#include <random>
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

namespace benchmark {

using namespace kvs;

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

KVS setupKVS(size_t setupElementsSize) {
  std::filesystem::create_directories(STORAGE_DIRECTORY_PATH);
  KVS kvs{};
  std::unordered_set<Key> keys;
  for (size_t i = 0; i < setupElementsSize; ++i) {
    kvs.add(generateNewRandomKey(keys), generateRandomValue());
    // x2
    // make random operation
  }
  std::cerr << "KVS set up finished\n";
  return kvs;
}

void clearUp() { std::filesystem::remove_all(STORAGE_DIRECTORY_PATH); }

void testRandomAccess(size_t setupElementsSize,
                      size_t benchmarkOperationsNumber,
                      double readOperationsRate) {
  KVS kvs = setupKVS(setupElementsSize);

  //
  // out
  //
  clearUp();
}

} // namespace benchmark

int main() {

  size_t setupElementsSize = 1000;
  size_t benchmarkOperationsNumber = 1e4;
  double readOperationsRate = 0.5;

  benchmark::testRandomAccess(setupElementsSize, benchmarkOperationsNumber,
                              readOperationsRate);

  /*double cacheAccessProbability = 0.7;
  benchmark::testCacheAccessWithProbability(
      setupElementsSize, benchmarkOperationsNumber, readOperationsRate,
      cacheAccessProbability);

  size_t fullMissesPeriod = 1000;
  benchmark::testCacheAccessWithPeriodicFullMisses(
      setupElementsSize, benchmarkOperationsNumber, readOperationsRate,
      fullMissesPeriod);
*/
  return 0;
}
