#pragma once

#include "KeyValueTypes.h"
#include <bitset>
#include <vector>

/**
 * @brief Bloom filter!
 * 
 */
class BloomFilter final {
public:
  BloomFilter() noexcept;

  /**
   * @brief Check if the Key is present.
   * 
   */
  bool checkExist(const Key& key) const noexcept;

  /**
   * @brief Add the Key to the filter.
   * 
   */
  void add(const Key& key) noexcept;

private:
  using seed_t = uint64_t;

  std::bitset<BLOOM_FILTER_SIZE> bitset;

  /**
   * @brief The seeds for different hash functions.
   * 
   */
  const std::vector<seed_t> seeds;
};
