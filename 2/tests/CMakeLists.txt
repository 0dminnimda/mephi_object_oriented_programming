cmake_minimum_required(VERSION 3.15)

project(test_cocktails)

set(CMAKE_CXX_STANDARD 17)

file(DOWNLOAD "https://raw.githubusercontent.com/doctest/doctest/master/doctest/doctest.h" "${CMAKE_CURRENT_BINARY_DIR}/doctest.h")

include_directories("${CMAKE_CURRENT_BINARY_DIR}")

add_compile_options(--coverage)
add_link_options(--coverage)

add_subdirectory(../cocktail cocktail)

add_executable(run_tests test_cocktail.cpp)
set_target_properties(run_tests PROPERTIES SUFFIX ".out")

target_link_libraries(run_tests PRIVATE cocktail_lib)
