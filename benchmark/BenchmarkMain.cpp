#include "KVS.h"

#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
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
std::uniform_int_distribution<uint32_t> opDistr(0, maxOpDistrRange);

constexpr size_t DEFAULT_RECENT_KEYS_SIZE = CACHE_MAP_SIZE;
double removeOperationsRate = 0.2; // between write and remove operations

struct Stats {
  uint32_t sumOperationsMicros = 0;
  uint32_t sumReadOperationsMicros = 0;
  uint32_t sumWriteOperationsMicros = 0;

  uint32_t operationsCnt = 0;
  uint32_t readOperationsCnt = 0;
  uint32_t writeOperationsCnt = 0;

  uint32_t maxReadOperationMicros = 0;
  uint32_t maxWriteOperationMicros = 0;

  uint32_t minReadOperationMicros = std::numeric_limits<uint32_t>::max();
  uint32_t minWriteOperationMicros = std::numeric_limits<uint32_t>::max();
};

void applyDuration(uint32_t duration, Stats& stats, uint8_t operationCode) {
  stats.sumOperationsMicros += duration;
  ++stats.operationsCnt;
  if (operationCode == 0) {
    ++stats.readOperationsCnt;
    stats.sumReadOperationsMicros += duration;
    stats.maxReadOperationMicros = duration > stats.maxReadOperationMicros
                                       ? duration
                                       : stats.maxReadOperationMicros;
    stats.minReadOperationMicros = duration < stats.minReadOperationMicros
                                       ? duration
                                       : stats.minReadOperationMicros;
  } else { // read or write operation
    ++stats.writeOperationsCnt;
    stats.sumWriteOperationsMicros += duration;
    stats.maxWriteOperationMicros = duration > stats.maxWriteOperationMicros
                                        ? duration
                                        : stats.maxWriteOperationMicros;
    stats.minWriteOperationMicros = duration < stats.minWriteOperationMicros
                                        ? duration
                                        : stats.minWriteOperationMicros;
  }
}

void printCSVFormatAverageStats(std::ostream& outs, const Stats& stats) {
  outs << stats.sumOperationsMicros / stats.operationsCnt << ","
       << stats.sumReadOperationsMicros / stats.readOperationsCnt << ","
       << stats.sumWriteOperationsMicros / stats.writeOperationsCnt << ",";
}

void printFullStats(std::ostream& outs, const Stats& stats) {
  outs << "Benchmark stats in microseconds:\n";

  outs << "avg operation = " << stats.sumOperationsMicros / stats.operationsCnt
       << "\n";
  outs << "avg read operation = "
       << stats.sumReadOperationsMicros / stats.readOperationsCnt << "\n";
  outs << "avg write operation = "
       << stats.sumWriteOperationsMicros / stats.writeOperationsCnt << "\n";

  outs << "min/max read operation = " << stats.minReadOperationMicros << " / "
       << stats.maxReadOperationMicros << "\n";
  outs << "min/max write operation = " << stats.minWriteOperationMicros << " / "
       << stats.maxWriteOperationMicros << "\n\n";
}

uint8_t generateRandomOperationCode(
    double readOperationsRate,
    double removeOperationsRate = removeOperationsRate) {
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
  //std::cerr << "KVS set up finished\n";
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
        std::chrono::duration_cast<std::chrono::microseconds>(end - begin)
            .count();
    applyDuration(duration, stats, operationCode);
  }

  printCSVFormatAverageStats(std::cerr, stats);
  clearUp();
}

void testCacheAccessWithProbability(size_t setupElementsSize,
                                    size_t benchmarkOperationsNumber,
                                    double readOperationsRate,
                                    double cacheAccessProbability) {
  KVS kvs = setupKVS(setupElementsSize);

  Stats stats{};
  std::vector<Key> recentKeys(DEFAULT_RECENT_KEYS_SIZE);
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
        std::chrono::duration_cast<std::chrono::microseconds>(end - begin)
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

  printCSVFormatAverageStats(std::cerr, stats);
  clearUp();
}

namespace disk {

constexpr size_t ENTRY_SIZE = VALUE_SIZE + KEY_SIZE;
std::string diskBenchmarkDirectoryPath = "../disk-benchmark/";
size_t filesNumber = 1e3;
size_t entriesInOneFile = 100;

void setUpDiskBenchmarkDirectory() {
  std::filesystem::create_directories(diskBenchmarkDirectoryPath);
  for (size_t i = 0; i < filesNumber; ++i) {
    std::ofstream file(diskBenchmarkDirectoryPath + std::to_string(i),
                       std::ios::out | std::ios::binary | std::ios::app);
    file.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    for (size_t j = 0; j < entriesInOneFile; ++j) {
      ByteArray bytes = generateRandomByteArray(ENTRY_SIZE);
      file.write(bytes.get(), ENTRY_SIZE);
    }
    file.close();
  }
}

void clearUpDiskBenchmarkDirectory() {
  std::filesystem::remove_all(diskBenchmarkDirectoryPath);
}

std::pair<std::string, size_t> generateEntryLocationFromKey(const Key& key) {
  assert(key.getBytes().length() >= 2 * sizeof(uint64_t));
  size_t firstPart =
      *reinterpret_cast<const uint64_t*>(key.getBytes().get()) % filesNumber;
  size_t secondPart = *reinterpret_cast<const uint64_t*>(key.getBytes().get() +
                                                         sizeof(uint64_t)) %
                      entriesInOneFile;
  return std::make_pair(diskBenchmarkDirectoryPath + std::to_string(firstPart),
                        secondPart * ENTRY_SIZE);
}

void testDiskOperations(size_t benchmarkOperationsNumber,
                        double readOperationsRate) {
  Stats stats{};
  ByteArray bytes = generateRandomByteArray(ENTRY_SIZE);

  for (size_t i = 0; i < benchmarkOperationsNumber; ++i) {
    Key key = generateRandomKey();
    auto [filePath, offset] = generateEntryLocationFromKey(key);
    uint8_t operationCode = generateRandomOperationCode(readOperationsRate);

    std::chrono::high_resolution_clock::time_point begin;
    std::chrono::high_resolution_clock::time_point end;
    if (operationCode == 0) { // read
      std::ifstream file(filePath, std::ios::in | std::ios::binary);
      file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
      file.seekg(offset);
      begin = std::chrono::high_resolution_clock::now();
      file.read(bytes.get(), ENTRY_SIZE);
      end = std::chrono::high_resolution_clock::now();
      file.close();
    } else { // write
      std::fstream file(filePath,
                        std::ios::in | std::ios::out | std::ios::binary);
      file.exceptions(std::fstream::failbit | std::fstream::badbit);
      file.seekp(offset);
      begin = std::chrono::high_resolution_clock::now();
      file.write(bytes.get(), ENTRY_SIZE);
      file.close();
      end = std::chrono::high_resolution_clock::now();
    }
    auto duration =
        std::chrono::duration_cast<std::chrono::microseconds>(end - begin)
            .count();
    applyDuration(duration, stats, operationCode);
  }
  printCSVFormatAverageStats(std::cout, stats);
}

} // namespace disk
} // namespace benchmark

void testAll(size_t benchmarkOperationsNumber) {
  size_t setupElementsSize = 10000;
  double readOperationsRate = 0.2;
  std::cout << benchmarkOperationsNumber << ",";
  benchmark::testRandomAccess(setupElementsSize, benchmarkOperationsNumber,
                              readOperationsRate);
  benchmark::testCacheAccessWithProbability(
      setupElementsSize, benchmarkOperationsNumber, readOperationsRate, 0.1);
  benchmark::testCacheAccessWithProbability(
      setupElementsSize, benchmarkOperationsNumber, readOperationsRate, 0.5);
  benchmark::testCacheAccessWithProbability(
      setupElementsSize, benchmarkOperationsNumber, readOperationsRate, 0.95);
  benchmark::disk::testDiskOperations(benchmarkOperationsNumber,
                                      readOperationsRate);
  std::cout << "\n";
}

int main() {

  benchmark::removeOperationsRate = 0.2; // between write and remove operations

  benchmark::disk::setUpDiskBenchmarkDirectory();
  size_t steps = 10;
  for (size_t i = 1; i < steps; ++i) {
    testAll(i * 10000);
  }
  benchmark::disk::clearUpDiskBenchmarkDirectory();

  return 0;
}
