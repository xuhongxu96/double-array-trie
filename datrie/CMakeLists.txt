add_library(datrie_builder INTERFACE datrie_builder.h)
target_link_libraries(datrie_builder INTERFACE dawg hashtrie)

add_executable(datrie_builder_tests datrie_builder_tests.cpp)
target_link_libraries(datrie_builder_tests PRIVATE datrie_builder)

add_library(datrie INTERFACE datrie.h)

