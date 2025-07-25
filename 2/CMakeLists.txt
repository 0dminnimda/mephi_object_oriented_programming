cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0091 NEW)

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)

project(cocktail_demo)

set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE ON)
set(CMAKE_CXX_STANDARD 20)

add_compile_options(-fsanitize=address -g -Wall -Werror)
add_link_options(-fsanitize=address -g -Wall -Werror)

add_subdirectory(cocktail)

add_executable(main main.cpp)
set_target_properties(main PROPERTIES SUFFIX ".out")

target_link_libraries(main cocktail_lib)

if (WIN32)
    set_target_properties(main PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    target_link_libraries(main -static)
else()
    # target_link_libraries(main stdc++)
endif()
