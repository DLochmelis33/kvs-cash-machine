cmake_minimum_required(VERSION 3.10)

project(cash-kvs)

set(TEST_PROG_NAME test)
set(BENCHMARK_PROG_NAME bench)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -g3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

set(KVS_SRC src/ByteArray.cpp src/KVSException.cpp src/Storage.cpp src/BloomFilter.cpp src/KeyValueTypes.cpp src/StorageHashTable.cpp src/Shard.cpp src/ShardBuilder.cpp src/CacheMap.cpp src/KVS.cpp)
set(TEST_SRC test/TestMain.cpp test/TestByteArray.cpp test/TestStorage.cpp test/TestBloomFilter.cpp test/TestStorageHashTable.cpp test/TestShard.cpp test/TestShardBuilder.cpp test/TestCacheMap.cpp test/TestKVS.cpp)
#set(TEST_SRC test/TestMain.cpp test/TestShardBuilder.cpp)
set(BENCHMARK_SRC benchmark/BenchmarkMain.cpp)

set(TEST_SRC_LIST ${KVS_SRC} ${TEST_SRC})
set(BENCHMARK_SRC_LIST ${KVS_SRC} ${BENCHMARK_SRC})

include_directories(include/)

add_executable(${TEST_PROG_NAME} ${TEST_SRC_LIST})
add_executable(${BENCHMARK_PROG_NAME} ${BENCHMARK_SRC_LIST})

add_subdirectory(../xxHash/cmake_unofficial/ ../../xxHash/build/ EXCLUDE_FROM_ALL)

target_link_libraries(${TEST_PROG_NAME} PRIVATE xxHash::xxhash)
target_link_libraries(${BENCHMARK_PROG_NAME} PRIVATE xxHash::xxhash)
