add_library(datrie_builder INTERFACE datrie_builder.h)
target_link_libraries(datrie_builder INTERFACE dawg hashtrie)
target_include_directories(datrie_builder INTERFACE .)

add_executable(datrie_builder_tests datrie_builder_tests.cpp)
target_link_libraries(datrie_builder_tests PRIVATE datrie_builder)

add_library(datrie INTERFACE)
target_include_directories(datrie INTERFACE .)

add_executable(datrie_tests datrie_tests.cpp)
target_link_libraries(datrie_tests PRIVATE datrie)
