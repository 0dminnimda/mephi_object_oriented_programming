cmake_minimum_required(VERSION 3.10)

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)

project(cocktail_lib)

set(CMAKE_CXX_STANDARD 17)

add_library(cocktail_lib STATIC cocktail.cpp)
target_include_directories(cocktail_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
