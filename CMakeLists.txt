cmake_minimum_required(VERSION 3.16)
project(Cow_trie)

set(CMAKE_CXX_STANDARD 20)


include_directories(
        ${PROJECT_SOURCE_DIR}/trie
        ${PROJECT_SOURCE_DIR}/utility
)

add_executable(Cow_trie
        test/trie_store_correctness_test.cpp
        trie/src.hpp
        utility/print_utility.hpp
)

target_link_libraries(Cow_trie PRIVATE pthread)
