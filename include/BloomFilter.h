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
   * @brief Check if the key is present.
   * 
   * @param object 
   */
  bool checkExist(const Key& key) const noexcept;

  /**
   * @brief Add the key to the filter.
   * 
   * @param object 
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
