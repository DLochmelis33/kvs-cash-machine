#pragma once

#include "KeyValueTypes.h"
#include <bitset>

template <typename T>
class BloomFilter {
  public:
    constexpr BloomFilter();

    bool checkExist(const T& object);

    void add(const T& object);

  private:
    std::bitset<BLOOM_FILTER_SIZE> bitset;

    // TODO: hash functions
};

// TODO: final classes