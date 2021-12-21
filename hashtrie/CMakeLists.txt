add_library(hashtrie INTERFACE hashtrie.h)
target_include_directories(hashtrie INTERFACE .)

add_executable(hashtrie_tests hashtrie_tests.cpp)
target_link_libraries(hashtrie_tests PRIVATE hashtrie)
