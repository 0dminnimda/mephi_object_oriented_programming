cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0091 NEW)

project(SparseMatrix)

set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE ON)
set(CMAKE_CXX_STANDARD 17)

add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)

add_executable(main main.cpp)
set_target_properties(main PROPERTIES SUFFIX ".out")

if (WIN32)
    set_target_properties(main PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    target_link_libraries(main -static)
else()
    # target_link_libraries(main stdc++)
endif()
