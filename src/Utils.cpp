#include "KeyValueTypes.h"
#include "xxh3.h"

namespace kvs {

hash_t hashKey(Key key, seed_t seed) noexcept {
    key_t k = key.get();
    // RV is guaranteed to be equivalent to uint64_t
    return XXH3_64bits_withSeed(&k, sizeof(k), seed);
}

} // namespace kvs