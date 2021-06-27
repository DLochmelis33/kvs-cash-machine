#include "BloomFilter.h"
#include <random>

namespace kvs::bloom_filter {

BloomFilter::BloomFilter() noexcept {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<seed_t> distr; // from 0 to type::max by default

    for (size_t i = 0; i < BLOOM_FILTER_HASH_FUNCTIONS_NUMBER; i++)
        seeds.push_back(distr(gen));
}

size_t mapToBitset(hash_t hash) { return hash % BLOOM_FILTER_SIZE; }

void BloomFilter::add(const Key& key) noexcept {
    for (seed_t seed : seeds)
        bitset.set(mapToBitset(hashKey(key, seed)));
}

bool BloomFilter::checkExist(const Key& key) const noexcept {
    for (seed_t seed : seeds)
        if (!bitset[mapToBitset(hashKey(key, seed))])
            return false;
    return true;
}

} // namespace kvs::bloom_filter