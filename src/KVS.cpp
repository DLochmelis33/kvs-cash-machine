#include "KVS.h"
#include "ShardBuilder.h"
#include <cassert>

namespace kvs {

using namespace utils;
using kvs::shard::ShardBuilder;

KVS::KVS() : shards(), cacheMap(CACHE_MAP_SIZE) {
  shards.reserve(SHARD_NUMBER);
  for (shard_index_t i = 0; i < SHARD_NUMBER; i++)
    shards.push_back(ShardBuilder::createShard(i));
}

void KVS::pushOperation(Entry displaced) {
  auto [key, ptr] = displaced;
  shard_index_t shardIndex = Shard::getShardIndex(key);
  switch (ptr.getType()) {

  case PtrType::DELETED: {
    shards[shardIndex].removeEntry(shardIndex, key);
    if (shards[shardIndex].isRebuildRequired(shardIndex)) {
      rebuildShard(shardIndex);
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

std::optional<Value> KVS::get(const Key& key) {
  shard_index_t shardIndex = Shard::getShardIndex(key);
  Ptr& ptr = cacheMap.get(key);
  switch (ptr.getType()) {

  case PtrType::PRESENT: {
    return std::optional<Value>(
        shards[shardIndex].readValueDirectly(shardIndex, ptr));
  }

  case PtrType::NONEXISTENT:
    [[fallthrough]];
  case PtrType::DELETED: {
    return std::optional<Value>();
  }

  case PtrType::EMPTY_PTR: {
    auto [newEntry, optValue] = shards[shardIndex].readValue(shardIndex, key);
    switch (newEntry.ptr.getType()) {

    case PtrType::PRESENT: {
      std::optional<Entry> displaced = cacheMap.putOrDisplace(newEntry);
      if (displaced.has_value())
        pushOperation(displaced.value());

      break;
    }

    case PtrType::EMPTY_PTR:
      [[fallthrough]];
    case PtrType::DELETED: {
      std::optional<Entry> displaced = cacheMap.putOrDisplace(
          Entry(newEntry.key, Ptr(PtrType::NONEXISTENT)));
      if (displaced.has_value())
        pushOperation(displaced.value());
    }

    case PtrType::NONEXISTENT: {
      assert(false && "returning NONEXISTENT Ptr from readValue() is illegal");
    }
    }

    return optValue;
  }
  }
}

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
      assert(false &&
             "returning PRESENT Ptr from removeEntry() makes no sense");
    }

    case PtrType::NONEXISTENT: {
      assert(false &&
             "returning NONEXISTENT Ptr from removeEntry() is illegal");
    }

    case PtrType::DELETED:
      [[fallthrough]];
    case PtrType::EMPTY_PTR: {
      std::optional<Entry> displaced = cacheMap.putOrDisplace(
          Entry(newEntry.key, Ptr(PtrType::NONEXISTENT)));
      if (displaced.has_value())
        pushOperation(displaced.value());

      break;
    }
    }
    break;
  }
  }
}

void KVS::rebuildShard(shard_index_t shardIndex) {
  auto [newShard, newEntries] =
      ShardBuilder::rebuildShard(shards[shardIndex], shardIndex, cacheMap);
  shards[shardIndex] = newShard;
  for (const Entry& newEntry : newEntries) {
    std::optional<Entry> displaced = cacheMap.putOrDisplace(newEntry);
    assert(!displaced.has_value());
  }
}

void KVS::clear() {
  // TODO
}

} // namespace kvs