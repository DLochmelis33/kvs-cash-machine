#pragma once

#include "ByteArray.h"
#include "xxh3.h"

#include <cstddef>
#include <cstdint>

namespace kvs::utils {

constexpr size_t VALUE_SIZE = 2048;
constexpr size_t KEY_SIZE = 16;
constexpr size_t CACHE_MAP_SIZE = 5000;
constexpr double MAP_LOAD_FACTOR = 1.5;
constexpr size_t SHARD_NUMBER = 4981;
constexpr double MAX_OUTDATED_RECORDS_LOAD_FACTOR = 0.5;
constexpr size_t BLOOM_FILTER_SIZE = 19;
constexpr size_t BLOOM_FILTER_HASH_FUNCTIONS_NUMBER = 2;
constexpr size_t SHARD_EXPECTED_SIZE = 23;
constexpr double STORAGE_HASH_TABLE_EXPANSION_FACTOR = 2;
constexpr double STORAGE_HASH_TABLE_LOAD_FACTOR = MAP_LOAD_FACTOR;
constexpr size_t STORAGE_HASH_TABLE_INITIAL_SIZE =
    SHARD_EXPECTED_SIZE * STORAGE_HASH_TABLE_LOAD_FACTOR;
constexpr size_t STORAGE_HASH_TABLE_MAX_SIZE =
    STORAGE_HASH_TABLE_EXPANSION_FACTOR * STORAGE_HASH_TABLE_INITIAL_SIZE;

const std::string STORAGE_DIRECTORY_PATH = "../data/";

using ptr_t = unsigned char;
using hash_t = XXH64_hash_t; // uint64_t
using seed_t = hash_t;

using shard_index_t = uint16_t;
using values_cnt_t = uint8_t;

class Key final {
public:
  explicit Key(ByteArray bytes = ByteArray{KEY_SIZE}) noexcept;

  ByteArray& getBytes() noexcept;

  const ByteArray& getBytes() const noexcept;

  bool operator==(const Key& other) const noexcept;

private:
  ByteArray bytes;
};

hash_t hashKey(const Key& key, seed_t seed = 0) noexcept;

class Value final {
public:
  explicit Value(ByteArray bytes = ByteArray(0));

  const ByteArray& getBytes() const noexcept;

  bool operator==(const Value& other) const noexcept;

private:
  ByteArray bytes;
};

struct KeyValue final {
  Key key;
  Value value;
};

/**
 * @brief A convenience class to check Ptr's type.
 * 
 * PRESENT - this Ptr points to an actual value
 * DELETED - this Ptr points to a value that existed before, but now is deleted from KVS
 * SYNC_DELETED (deprecated, see NONEXSITENT) - this Ptr points to nowhere, was lazily deleted in CacheMap and then synced with the Shard
 * NONEXISTENT - this Ptr points to nowhere, but it was PRESENT before and the current corresponding Key is valid
 * EMPTY_PTR - this Ptr is equivalent to NULL
 *
 * NONEXISTENT is forbidden in StorageHashTable
 *
 */
enum class PtrType {
  PRESENT,
  DELETED,
  EMPTY_PTR,
  NONEXISTENT /*, SYNC_DELETED */
};

/**
 * @brief A pointer determining the position of the associated value in the values file. Also stores
 * information whether the associated value is present or deleted.
 *
 */
class Ptr final {

public:
  /**
   * @brief Reserved values for specific pointers.
   *
   */
  static constexpr ptr_t EMPTY_PTR_V = 0b01111111;
  static constexpr ptr_t NONEXISTENT_V = 0b01111110;
  // static constexpr ptr_t SYNC_DELETED_V = 0b01111101;

  /**
   * @brief Construct a new Ptr from \b raw \b data.
   * 
   * To create a Ptr to some index, use Ptr(size_t, bool).
   * 
   * @param ptr 
   */
  explicit Ptr(ptr_t ptr) noexcept;

  /** 
   * @brief Construct a new Ptr pointing to the given offset and with the given isPresent flag.
   * 
   */
  explicit Ptr(size_t offset, bool isPresent) noexcept;

  /**
   * @brief Construct a new empty Ptr. // TODO docs
   * 
   */
  explicit Ptr(PtrType type = PtrType::EMPTY_PTR) noexcept;

  /**
   * @brief Get the actual index stored in this pointer.
   *
   */
  size_t get() const noexcept;

  // TODO docs
  size_t getOffset() const noexcept;

  /**
   * @brief Get the raw pointer data (including control bits).
   * 
   */
  ptr_t getRaw() const noexcept;

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

  // TODO docs
  PtrType getType() const noexcept;

  bool operator==(const Ptr& other) const noexcept;
  bool operator!=(const Ptr& other) const noexcept;

private:
  static constexpr ptr_t CONTROL_MASK = 0b10000000;

  ptr_t ptr;
};

/**
 * @brief \b DO \b NOT \b CHANGE \b THIS \b !!!!!!!!!
 * 
 */
static Ptr EMPTY_PTR = Ptr();

/**
 * @brief A wrapper for entries in hash tables (i.e. CacheMap and StorageHashTable).
 *
 */
struct Entry final {

  Entry(const Key& key_ = Key(), const Ptr& ptr_ = EMPTY_PTR) noexcept
      : key(key_),
        ptr(ptr_) {}

  Key key;
  Ptr ptr;

  bool operator==(const Entry& other) const noexcept;
};

} // namespace kvs::utils
