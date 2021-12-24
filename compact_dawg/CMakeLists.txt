add_library(compact_dawg INTERFACE compact_dawg.h)
target_include_directories(compact_dawg INTERFACE .)

add_executable(compact_dawg_tests compact_dawg_tests.cpp)
target_link_libraries(compact_dawg_tests PRIVATE compact_dawg)
