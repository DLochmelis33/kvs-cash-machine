#pragma once

#include "KeyValueTypes.h"
#include <bitset>

template <typename T>
/**
 * @brief Bloom filter!
 * 
 */
class BloomFilter final {
public:
  BloomFilter() noexcept;

  /**
   * @brief Check if the object is present.
   * 
   * @param object 
   */
  bool checkExist(const T& object) const noexcept;

  /**
   * @brief Add the object to the filter.
   * 
   * @param object 
   */
  void add(const T& object) noexcept;

private:
  using seed_t = uint64_t;

  std::bitset<BLOOM_FILTER_SIZE> bitset;

  /**
   * @brief The seeds for different hash functions.
   * 
   */
  const std::vector<seed_t> seeds;
};
