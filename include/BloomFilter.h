#pragma once

#include "KeyValueTypes.h"
#include <bitset>
#include <vector>

namespace kvs::bloom_filter {

using namespace kvs::utils;

/**
 * @brief Bloom filter!!!
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
  std::bitset<BLOOM_FILTER_SIZE> bitset;

  /**
     * @brief The seeds for different hash functions.
     *
     */
  std::vector<seed_t> seeds;
};

} // namespace kvs::bloom_filter
