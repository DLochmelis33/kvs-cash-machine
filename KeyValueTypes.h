#pragma once

#include <cstddef>

constexpr size_t VALUE_SIZE = 2048;
constexpr size_t KEY_SIZE = 16;
constexpr size_t CACHE_MAP_SIZE = 5000;
constexpr double MAP_LOAD_FACTOR = 1.5;
constexpr size_t SHARD_NUMBER = 4981;
constexpr size_t BLOOM_FILTER_SIZE = 5;                  // TODO
constexpr size_t BLOOM_FILTER_HASH_FUNCTIONS_NUMBER = 5; // TODO

using key_t = __uint128_t;
using value_t = char*;
using ptr_t = char;

class Key {

  public:
    key_t get();
};

class Value {

  public:
    value_t get();
};

struct KeyValue {
    Key key;
    Value value;
};

class Ptr {

  public:
    ptr_t get() const;
    bool isValueDeleted() const;

    void setValueExists(bool exists);
};
