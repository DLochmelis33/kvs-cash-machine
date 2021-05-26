#pragma once

#include "KeyValueTypes.h"
#include <bitset>

template <typename T>
final class BloomFilter {
public:
  BloomFilter();

  bool checkExist(const T& object) const;

  void add(const T& object);

  void clear();

private:
  using seed_t = uint64_t;

  std::bitset<BLOOM_FILTER_SIZE> bitset;
  const std::vector<seed_t> seeds;
};
