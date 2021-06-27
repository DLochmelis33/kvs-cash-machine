#pragma once

#define XXH_INLINE_ALL

#include "ByteArray.h"

#include <cstddef>

namespace kvs {

constexpr size_t VALUE_SIZE = 2048;
constexpr size_t KEY_SIZE = 16;
constexpr size_t CACHE_MAP_SIZE = 5000;
constexpr double MAP_LOAD_FACTOR = 1.5;
constexpr size_t SHARD_NUMBER = 4981;
constexpr size_t BLOOM_FILTER_SIZE = 5000;               // TODO
constexpr size_t BLOOM_FILTER_HASH_FUNCTIONS_NUMBER = 5; // TODO

using key_t = __uint128_t;
using value_t = char*;
using ptr_t = unsigned char;
using hash_t = uint64_t;
using seed_t = hash_t;

class Key final {
  public:
    Key(key_t key_) noexcept
            : key(key_) {}

    key_t get() const noexcept { return key; }

    bool operator==(const Key& other) const noexcept { return key == other.key; }

  private:
    key_t key;
};

hash_t hashKey(Key key, seed_t seed) noexcept;

class Value final {
  public:
    value_t get() const noexcept { return value; }

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
    static constexpr ptr_t EMPTY_PTR = 0b11111111;

    explicit Ptr(ptr_t ptr) noexcept;

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

  private:
    ptr_t ptr;
};

/**
 * @brief A wrapper for entries in hash tables (i.e. CacheMap and StorageHashTable).
 *
 */
struct Entry final {
    Key key;
    Ptr ptr;
};

} // namespace kvs
