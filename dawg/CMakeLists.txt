add_library(dawg INTERFACE dawg.h)
target_include_directories(dawg INTERFACE .)

add_executable(dawg_tests dawg_tests.cpp)
target_link_libraries(dawg_tests PRIVATE dawg)
