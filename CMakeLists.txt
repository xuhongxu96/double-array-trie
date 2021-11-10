cmake_minimum_required(VERSION 3.20)

project("double-array-trie")

set(CMAKE_CXX_STANDARD 20)

include_directories(thirdparty)
add_compile_definitions(BOOST_UT_DISABLE_MODULE)

add_subdirectory(hashtrie)
add_subdirectory(datrie)
