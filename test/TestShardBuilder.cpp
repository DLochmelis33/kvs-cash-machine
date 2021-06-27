#include "ShardBuilder.h"
#include "Storage.h"
#include "StorageHashTable.h"
#include "doctest.h"

#include <filesystem>

using namespace kvs::shard;
using namespace kvs::storage;
using kvs::storage_hash_table::StorageHashTable;

namespace test_kvs::shard_builder {

const std::string testDirectoryPath = "../.test-data/test-shard-builder/";

void setUpTestDirectory() {
  std::filesystem::create_directories(testDirectoryPath);
  Shard::storageDirectoryPath = testDirectoryPath;
}

void clearTestDirectory() { std::filesystem::remove_all(testDirectoryPath); }

TEST_CASE("test ShardBuilder") {
  setUpTestDirectory();

  SUBCASE("test createShard") {
    shard_index_t shardIndex = 65;
    Shard shard = ShardBuilder::createShard(shardIndex);

    std::string valuesFilePath = Shard::getValuesFilePath(shardIndex);
    REQUIRE(std::filesystem::exists(valuesFilePath));
    CHECK(std::filesystem::file_size(valuesFilePath) == 0);

    std::string hashTableFilePath =
        Shard::getStorageHashTableFilePath(shardIndex);
    REQUIRE(std::filesystem::exists(hashTableFilePath));
    CHECK(StorageHashTable{readFile(hashTableFilePath)}.getEntries().size() ==
          0);

    CHECK_FALSE(shard.isRebuildRequired(shardIndex));
  }

  clearTestDirectory();
}

} // namespace test_kvs::shard_builder
