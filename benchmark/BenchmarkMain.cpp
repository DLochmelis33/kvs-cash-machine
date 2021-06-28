#include "KVS.h"

#include <chrono>
#include <cstdint>
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
constexpr uint32_t maxOpDistrRange = 1e6;
constexpr double defaultRemoveOperationsRate =
    0.4; // from write and remove operations
std::uniform_int_distribution<uint32_t> opDistr(0, maxOpDistrRange);

constexpr size_t defaultRecentKeysSize = 500;

struct Stats {
  uint64_t sumOperationsNanos = 0;
  uint64_t sumReadOperationsNanos = 0;
  uint64_t sumWriteOperationsNanos = 0;

  uint64_t operationsCnt = 0;
  uint64_t readOperationsCnt = 0;
  uint64_t writeOperationsCnt = 0;

  uint64_t maxReadOperationNanos = 0;
  uint64_t maxWriteOperationNanos = 0;

  uint64_t minReadOperationNanos = std::numeric_limits<uint64_t>::max();
  uint64_t minWriteOperationNanos = std::numeric_limits<uint64_t>::max();
};

void applyDuration(uint64_t duration, Stats& stats, uint8_t operationCode) {
  stats.sumOperationsNanos += duration;
  ++stats.operationsCnt;
  if (operationCode == 0) {
    ++stats.readOperationsCnt;
    stats.sumReadOperationsNanos += duration;
    stats.maxReadOperationNanos = duration > stats.maxReadOperationNanos
                                      ? duration
                                      : stats.maxReadOperationNanos;
    stats.minReadOperationNanos = duration < stats.minReadOperationNanos
                                      ? duration
                                      : stats.minReadOperationNanos;
  } else if (operationCode == 2) {
    ++stats.writeOperationsCnt;
    stats.sumWriteOperationsNanos += duration;
    stats.maxWriteOperationNanos = duration > stats.maxWriteOperationNanos
                                       ? duration
                                       : stats.maxWriteOperationNanos;
    stats.minWriteOperationNanos = duration < stats.minWriteOperationNanos
                                       ? duration
                                       : stats.minWriteOperationNanos;
  }
}

void printAverageStats(std::ostream& outs, const Stats& stats) {
  outs << "Benchmark stats in nanos:\n";

  outs << "avg operation = " << stats.sumOperationsNanos / stats.operationsCnt
       << "\n";
  outs << "avg read operation = "
       << stats.sumReadOperationsNanos / stats.readOperationsCnt << "\n";
  outs << "avg write operation = "
       << stats.sumWriteOperationsNanos / stats.writeOperationsCnt << "\n";

  outs << "min/max read operation = " << stats.minReadOperationNanos << " / "
       << stats.maxReadOperationNanos << "\n";
  outs << "min/max write operation = " << stats.minWriteOperationNanos << " / "
       << stats.maxWriteOperationNanos << "\n\n";
}

uint8_t generateRandomOperationCode(
    double readOperationsRate,
    double removeOperationsRate = defaultRemoveOperationsRate) {
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

Key generateRandomKeyWithCacheAccessProbability(std::vector<Key>& recentKeys,
                                                size_t size,
                                                double cacheAccessProbability) {
  uint32_t num = opDistr(gen);
  uint32_t maxCacheNum = maxOpDistrRange * cacheAccessProbability;
  if (num > maxCacheNum || size == 0) {
    return generateRandomKey();
  }
  return recentKeys[num % size];
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
    // TODO: (?) make random operation
  }
  std::cerr << "KVS set up finished\n";
  return kvs;
}

void clearUp() { std::filesystem::remove_all(STORAGE_DIRECTORY_PATH); }

void testRandomAccess(size_t setupElementsSize,
                      size_t benchmarkOperationsNumber,
                      double readOperationsRate) {
  KVS kvs = setupKVS(setupElementsSize);

  Stats stats{};

  for (size_t i = 0; i < benchmarkOperationsNumber; ++i) {
    Key key = generateRandomKey();
    uint8_t operationCode = generateRandomOperationCode(readOperationsRate);

    auto begin = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::time_point end;
    switch (operationCode) {
    case 0: {
      std::optional<Value> value = kvs.get(key);
      end = std::chrono::high_resolution_clock::now();
      break;
    }
    case 1: {
      kvs.remove(key);
      end = std::chrono::high_resolution_clock::now();
      break;
    }
    case 2: {
      kvs.add(key, generateRandomValue());
      end = std::chrono::high_resolution_clock::now();
      break;
    }
    }
    auto duration =
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin)
            .count();
    applyDuration(duration, stats, operationCode);
  }

  printAverageStats(std::cerr, stats);
  clearUp();
}

void testCacheAccessWithProbability(size_t setupElementsSize,
                                    size_t benchmarkOperationsNumber,
                                    double readOperationsRate,
                                    double cacheAccessProbability) {
  KVS kvs = setupKVS(setupElementsSize);

  Stats stats{};
  std::vector<Key> recentKeys(defaultRecentKeysSize);
  size_t recentKeysSize = 0;
  size_t recentKeysPos = 0;

  for (size_t i = 0; i < benchmarkOperationsNumber; ++i) {
    Key key = generateRandomKeyWithCacheAccessProbability(
        recentKeys, recentKeysSize, cacheAccessProbability);
    uint8_t operationCode = generateRandomOperationCode(readOperationsRate);

    auto begin = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::time_point end;
    switch (operationCode) {
    case 0: {
      std::optional<Value> value = kvs.get(key);
      end = std::chrono::high_resolution_clock::now();
      break;
    }
    case 1: {
      kvs.remove(key);
      end = std::chrono::high_resolution_clock::now();
      break;
    }
    case 2: {
      kvs.add(key, generateRandomValue());
      end = std::chrono::high_resolution_clock::now();
      break;
    }
    }
    auto duration =
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin)
            .count();
    applyDuration(duration, stats, operationCode);

    if (recentKeysSize < recentKeys.size()) {
      recentKeys[recentKeysSize] = key;
      ++recentKeysSize;
      ++recentKeysPos;
    } else {
      if (recentKeysPos >= recentKeysSize) {
        recentKeysPos %= recentKeysSize;
      }
      recentKeys[recentKeysPos] = key;
      ++recentKeysPos;
    }
  }

  printAverageStats(std::cerr, stats);
  clearUp();
}

} // namespace benchmark

int main() {

  size_t setupElementsSize = 1000;
  size_t benchmarkOperationsNumber = 1e4;
  double readOperationsRate = 0.5;

  benchmark::testRandomAccess(setupElementsSize, benchmarkOperationsNumber,
                              readOperationsRate);

  double cacheAccessProbability = 0.7;
  benchmark::testCacheAccessWithProbability(
      setupElementsSize, benchmarkOperationsNumber, readOperationsRate,
      cacheAccessProbability);

  /*size_t fullMissesPeriod = 1000;
  benchmark::testCacheAccessWithPeriodicFullMisses(
      setupElementsSize, benchmarkOperationsNumber, readOperationsRate,
      fullMissesPeriod);
*/
  return 0;
}
