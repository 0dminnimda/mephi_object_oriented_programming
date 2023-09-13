cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0091 NEW)

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)

project(cocktail_lib)

set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE ON)
set(CMAKE_CXX_STANDARD 17)

add_library(cocktail_lib STATIC cocktail.cpp)
target_include_directories(cocktail_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if (WIN32)
    set_target_properties(cocktail_lib PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    target_link_libraries(cocktail_lib -static)
else()
    # target_link_libraries(cocktail_lib stdc++)
endif()
