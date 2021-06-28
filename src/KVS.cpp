#include "KVS.h"
#include "ShardBuilder.h"
#include <cassert>

namespace kvs {

using namespace utils;
using kvs::shard::ShardBuilder;

KVS::KVS() : shards(), cacheMap(CACHE_MAP_SIZE) {
  shards.reserve(SHARD_NUMBER);
  for (shard_index_t i = 0; i < SHARD_NUMBER; i++)
    shards[i] = ShardBuilder::createShard(i);
}

void KVS::pushOperation(Entry displaced) {
  auto [key, ptr] = displaced;
  shard_index_t shardIndex = Shard::getShardIndex(key);
  switch (ptr.getType()) {

  case PtrType::DELETED: {
    shards[shardIndex].removeEntry(shardIndex, key);
    if (shards[shardIndex].isRebuildRequired(shardIndex)) {
      // TODO
      //   rebuildShard(shardIndex);
    }
    break;
  }

  case PtrType::PRESENT:
    [[fallthrough]];
  case PtrType::NONEXISTENT:
    break;
  case PtrType::EMPTY_PTR: {
    assert(false && "pushing EMPTY_PTR makes no sense");
  }
  }
}

void KVS::add(const Key& key, const Value& value) {
  shard_index_t shardIndex = Shard::getShardIndex(key);
  Ptr& ptr = cacheMap.get(key);
  switch (ptr.getType()) {

  case PtrType::PRESENT: {
    shards[shardIndex].writeValueDirectly(shardIndex, ptr, value);
    break;
  }

  case PtrType::DELETED: {
    shards[shardIndex].writeValueDirectly(shardIndex, ptr, value);
    ptr.setValuePresent(true);
    shards[shardIndex].incrementAliveValuesCnt();
    break;
  }

  case PtrType::NONEXISTENT:
    [[fallthrough]];
  case PtrType::EMPTY_PTR: {
    std::optional<Entry> displaced = cacheMap.putOrDisplace(
        shards[shardIndex].writeValue(shardIndex, key, value));
    if (displaced.has_value())
      pushOperation(displaced.value());

    break;
  }
  }
}

std::optional<Value> KVS::get(const Key& key) {}

void KVS::remove(const Key& key) {
  shard_index_t shardIndex = Shard::getShardIndex(key);
  Ptr& ptr = cacheMap.get(key);
  switch (ptr.getType()) {
  case PtrType::PRESENT: {
    // lazy deletion
    ptr.setValuePresent(false);
    break;
  }

  case PtrType::DELETED:
    break;

  case PtrType::NONEXISTENT:
    [[fallthrough]];
  case PtrType::EMPTY_PTR: {
    Entry newEntry = shards[shardIndex].removeEntry(shardIndex, key);
    switch (newEntry.ptr.getType()) {

    case PtrType::PRESENT: {
      std::optional<Entry> displaced = cacheMap.putOrDisplace(newEntry);
      if (displaced.has_value())
        pushOperation(displaced.value());
    }

    case PtrType::DELETED: {
      std::optional<Entry> displaced =
          cacheMap.putOrDisplace(Entry(newEntry.key, Ptr()));
      if (displaced.has_value())
        pushOperation(displaced.value());
    }

    case PtrType::NONEXISTENT: {
      assert(false && "NONEXISTENT pointer inside shard is illegal");
    }

    case PtrType::EMPTY_PTR: {
    }
    }
    break;
  }
  }
}

} // namespace kvs