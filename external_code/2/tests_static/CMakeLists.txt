cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

find_package(Catch2 2 REQUIRED)

add_executable(test_static_testing testing_static.cpp)

target_link_libraries(test_static_testing stack_static test Catch2::Catch2)