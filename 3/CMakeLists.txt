cmake_minimum_required(VERSION 3.16)
project(EpicLab3Game LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)

set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE ON)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

if(NOT ANDROID)
    include(FetchContent)
    FetchContent_Declare(SFML
        SOURCE_DIR ../../SFML
    )
    FetchContent_MakeAvailable(SFML)
endif()

if (CMAKE_BUILD_TYPE EQUAL "DEBUG")
    add_compile_definitions(DEBUG)
endif()

add_executable(main src/main.cpp src/game.cpp)
set_target_properties(main PROPERTIES SUFFIX ".out")
target_link_libraries(main PRIVATE sfml-window sfml-system sfml-graphics sfml-audio)

if(WIN32)
    add_custom_command(
        TARGET main
        COMMENT "Copy OpenAL DLL"
        PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${SFML_SOURCE_DIR}/extlibs/bin/$<IF:$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>,x64,x86>/openal32.dll $<TARGET_FILE_DIR:main>
        VERBATIM)
endif()

install(TARGETS main)
