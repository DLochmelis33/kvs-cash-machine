#pragma once

#include "ByteArray.h"
#include <cstddef>

namespace kvs::utils {

constexpr size_t VALUE_SIZE = 2048;
constexpr size_t KEY_SIZE = 16;
constexpr size_t CACHE_MAP_SIZE = 5000;
constexpr double MAP_LOAD_FACTOR = 1.5;
constexpr size_t SHARD_NUMBER = 4981;
constexpr size_t BLOOM_FILTER_SIZE = 5000; // TODO
constexpr size_t BLOOM_FILTER_HASH_FUNCTIONS_NUMBER = 5; // TODO
constexpr double TABLE_EXPANSION_FACTOR = 2;
constexpr size_t TABLE_MAX_SIZE = 50; // TODO

using key_t = __uint128_t;
using value_t = char*;
using ptr_t = unsigned char;
using hash_t = uint64_t;
using seed_t = hash_t;

class Key final {
public:
  Key(key_t key_) noexcept;

  key_t get() const noexcept;

  bool operator==(const Key& other) const noexcept;

private:
  key_t key;
};

hash_t hashKey(Key key, seed_t seed = 0) noexcept;

class Value final {
public:
  value_t get() const noexcept;

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
  /**
   * @brief A reserved value resembling a pointer to nowhere.
   *
   */
  static constexpr ptr_t EMPTY_PTR_V = 0b01111111;

  /**
   * @brief Construct a new Ptr from \b raw \b data.
   * 
   * To create a Ptr to some index, use Ptr(size_t, bool).
   * 
   * @param ptr 
   */
  explicit Ptr(ptr_t ptr) noexcept;

  /**
   * @brief Construct a new Ptr pointing to the given index and with the given isPresent flag.
   * 
   * @param index 
   * @param isPresent 
   */
  explicit Ptr(size_t index, bool isPresent);

  /**
   * @brief Get the real value of this pointer.
   *
   */
  ptr_t get() const noexcept;

  /**
   * @brief Check if this Ptr points to a present or deleted Value.
   *
   */
  bool isValuePresent() const noexcept;

  /**
   * @brief Set state for the Value associated with this Ptr.
   *
   * @param isPresent
   */
  void setValuePresent(bool isPresent) noexcept;

  bool operator==(const Ptr& other) noexcept;
  bool operator!=(const Ptr& other) noexcept;

private:
  static constexpr ptr_t CONTROL_MASK = 0b10000000;

  ptr_t ptr;
};

const Ptr EMPTY_PTR = Ptr(Ptr::EMPTY_PTR_V);

/**
 * @brief A wrapper for entries in hash tables (i.e. CacheMap and StorageHashTable).
 *
 */
struct Entry final {

  Entry(Key key_ = Key(0), Ptr ptr_ = EMPTY_PTR) noexcept
      : key(key_),
        ptr(ptr_) {}

  Key key;
  Ptr ptr;
};

} // namespace kvs::utils
