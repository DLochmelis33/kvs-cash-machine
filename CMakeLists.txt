cmake_minimum_required(VERSION 3.10)

project(cash-kvs)

set(TEST_PROG_NAME test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

set(KVS_SRC src/ByteArray.cpp)
set(TEST_SRC test/TestMain.cpp test/TestByteArray.cpp)
set(SRC_LIST ${KVS_SRC} ${TEST_SRC})

include_directories(include/)

add_executable(${TEST_PROG_NAME} ${SRC_LIST})

add_subdirectory(../xxHash/cmake_unofficial/ ../../xxHash/build/ EXCLUDE_FROM_ALL)

target_link_libraries(${TEST_PROG_NAME} PRIVATE xxHash::xxhash)
