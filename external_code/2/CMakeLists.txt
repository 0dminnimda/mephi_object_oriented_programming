
cmake_minimum_required(VERSION 3.16)

project(oop_lab2)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_PROFILE --coverage)

option(DYNAMIC OFF)
if (DYNAMIC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D DYNAMIC -Wall -Wextra")
    add_subdirectory(StackLib)
    #add_subdirectory(tests)
    message("DYNAMIC\n")
    #link_libraries(gcov)
    #add_compile_options(-fprofile-arcs -ftest-coverage)
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D STATIC -Wall -Wextra")
    add_subdirectory(StackStaticLib)
    #add_subdirectory(tests_static)
    message("STATIC\n")
    #link_libraries(gcov)
    #add_compile_options(-fprofile-arcs -ftest-coverage)
endif()
add_subdirectory(TestLib)



add_executable(main ligma.cpp help.cpp menu.cpp)
if (DYNAMIC)
    target_link_libraries(main stack test)
else()
    target_link_libraries(main stack_static test)
endif()