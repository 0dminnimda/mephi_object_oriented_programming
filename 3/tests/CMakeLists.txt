include(${CMAKE_CURRENT_SOURCE_DIR}/../CMakeLists.txt)

cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0091 NEW)

project(tests)

set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE ON)
set(CMAKE_CXX_STANDARD 20)

# include_directories("${CMAKE_CURRENT_BINARY_DIR}")

add_compile_options(-fprofile-instr-generate -fcoverage-mapping)
add_link_options(-fprofile-instr-generate -fcoverage-mapping)

# add_subdirectory(.. EpicLab3Game)

add_executable(run_tests ${PROJ_ROOT}/tests/tests.cpp)
set_target_properties(run_tests PROPERTIES SUFFIX ".out")
target_link_libraries(run_tests sfml-window sfml-system sfml-graphics sfml-audio ${Boost_LIBRARIES})
