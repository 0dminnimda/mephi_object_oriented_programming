cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0091 NEW)

project(test_cocktails)

set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE ON)
set(CMAKE_CXX_STANDARD 20)

file(DOWNLOAD "https://raw.githubusercontent.com/doctest/doctest/master/doctest/doctest.h" "${CMAKE_CURRENT_BINARY_DIR}/doctest.h")

include_directories("${CMAKE_CURRENT_BINARY_DIR}")

add_compile_options(-fprofile-instr-generate -fcoverage-mapping)
add_link_options(-fprofile-instr-generate -fcoverage-mapping)

add_subdirectory(../cocktail cocktail)

add_executable(run_tests test_cocktail.cpp)
set_target_properties(run_tests PROPERTIES SUFFIX ".out")

target_link_libraries(run_tests cocktail_lib)

if (WIN32)
    set_target_properties(run_tests PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    target_link_libraries(run_tests -static)
else()
    # target_link_libraries(run_tests stdc++)
endif()
