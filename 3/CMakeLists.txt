cmake_minimum_required(VERSION 3.16)
project(EpicLab3Game LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)

set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE ON)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/libs)
# option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

# set(Boost_USE_STATIC_LIBS ON)
# set(Boost_USE_MULTITHREADED OFF)
# set(Boost_USE_STATIC_RUNTIME ON)
find_package(Boost 1.45.0 COMPONENTS serialization filesystem)

if(NOT Boost_FOUND)
    message( SEND_ERROR "You need to have boost to compile this." )
endif()
include_directories(${Boost_INCLUDE_DIRS}) 

add_compile_options(-fPIC)
add_link_options(-fPIC)

if(NOT ANDROID)
    include(FetchContent)
    FetchContent_Declare(SFML
        SOURCE_DIR ${PROJ_ROOT}/../SFML
    )
    FetchContent_MakeAvailable(SFML)
endif()

if(NOT WIN32)
    # add_compile_options(-fsanitize=address,undefined)
    # add_link_options(-fsanitize=address,undefined)
    add_compile_options(-fsanitize=undefined)
    add_link_options(-fsanitize=undefined)
    add_compile_options(-fsanitize=thread)
    add_link_options(-fsanitize=thread)
    add_compile_options(-g)
    add_link_options(-g)
endif()

add_compile_definitions(TOML_HEADER_ONLY=0)

if (CMAKE_BUILD_TYPE EQUAL "DEBUG")
    add_compile_definitions(DEBUG)
endif()

if(NOT PROJ_NOT_BUILD_MAIN)
    set(MY_LIBS sfml-window sfml-system sfml-graphics sfml-audio ${Boost_LIBRARIES})

    include(items.cmake)

    foreach(SOURCE_FILE ${LIB_FILES})
        # filename without an extension
        get_filename_component(LIB_NAME ${SOURCE_FILE} NAME_WE)

        add_library(${LIB_NAME} SHARED ${SOURCE_FILE})
        set_target_properties(${LIB_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
        target_link_libraries(${LIB_NAME} ${MY_LIBS})
    endforeach()

    add_executable(main ${PROJ_ROOT}/src/toml_impl.cpp ${PROJ_ROOT}/src/game.cpp ${PROJ_ROOT}/src/main.cpp)
    set_target_properties(main PROPERTIES ENABLE_EXPORTS ON)
    # set_target_properties(main PROPERTIES POSITION_INDEPENDENT_CODE ON)
    set_target_properties(main PROPERTIES SUFFIX ".out")
    target_link_libraries(main ${MY_LIBS})
endif()

if(WIN32)
    add_custom_command(
        TARGET main
        COMMENT "Copy OpenAL DLL"
        PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${SFML_SOURCE_DIR}/extlibs/bin/$<IF:$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>,x64,x86>/openal32.dll $<TARGET_FILE_DIR:main>
        VERBATIM)
endif()
