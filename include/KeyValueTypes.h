#pragma once

#include <cstddef>

constexpr size_t VALUE_SIZE = 2048;
constexpr size_t KEY_SIZE = 16;
constexpr size_t CACHE_MAP_SIZE = 5000;
constexpr double MAP_LOAD_FACTOR = 1.5;
constexpr size_t SHARD_NUMBER = 4981;
constexpr size_t BLOOM_FILTER_SIZE = 5; // TODO
constexpr size_t BLOOM_FILTER_HASH_FUNCTIONS_NUMBER = 5; // TODO

using key_t = __uint128_t;
using value_t = char*;
using ptr_t = char;

class Key final {
public:
  key_t get() noexcept;

private:
  key_t key;
};

class Value final {
public:
  value_t get() noexcept;

private:
  value_t value;
};

struct KeyValue final {
  Key key;
  Value value;
};

/**
 * @brief A pointer determining the position of the associated value in the values file. Also stores
 * information whether the associated value is present or deleted.
 *
 */
class Ptr final {

public:
  static constexpr ptr_t EMPTY_PTR = 0b11111111;

  ptr_t get() const noexcept;

  /**
     * @brief Check if this Ptr points to a present or deleted value.
     *
     */
  bool isValuePresent() const noexcept;

  /**
     * @brief Set state for the value associated with this Ptr.
     *
     * @param isPresent
     */
  void setValuePresent(bool isPresent) noexcept;

private:
  ptr_t ptr;
};

struct Entry final {
  Key key;
  Ptr ptr;
};
