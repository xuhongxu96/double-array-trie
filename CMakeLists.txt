cmake_minimum_required(VERSION 3.20)

project("double-array-trie")

set(CMAKE_CXX_STANDARD 20)

include_directories(thirdparty common concepts)
add_compile_definitions(
    BOOST_UT_DISABLE_MODULE 

    DATA_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/data/\" 

    ASSERT_CONCEPT
)

add_subdirectory(hashtrie)
add_subdirectory(dawg)
add_subdirectory(compact_dawg)
add_subdirectory(datrie)
add_subdirectory(comparison)
