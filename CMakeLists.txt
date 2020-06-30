cmake_minimum_required(VERSION 3.1...3.13)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(project
        VERSION 0.1.0
        DESCRIPTION "A clean SDL2 Vulkan project"
        LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)


# Options

option(PACKAGE_TESTS "Build the tests" OFF)


# Generic

## C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

## Interprocedural Optimization
include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported)

## CCache
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()


# Dependencies

## GoogleTest
include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.8.0
)

FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
    FetchContent_Populate(googletest)
    add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
endif()

## SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})

## Vulkan
find_package(vulkan REQUIRED)


# Project

set(PROJECT_DEPS ${SDL2_LIBRARY} Vulkan::Vulkan)

include_directories(include)

set(${PROJECT_NAME}_SOURCES
    src/${PROJECT_NAME}.cpp
)
add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SOURCES})
if(ipo_supported)
    set_target_properties(${PROJECT_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()
target_link_libraries(${PROJECT_NAME} ${PROJECT_DEPS})
install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(FILES ${PROJECT_NAME}.h DESTINATION include)

set(example_SOURCES
    examples/main.cpp
)
add_executable(example ${example_SOURCES})
if(ipo_supported)
    set_target_properties(example PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()
target_link_libraries(example ${PROJECT_NAME} ${PROJECT_DEPS})
install(TARGETS example DESTINATION bin)


# Tests

if(PACKAGE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
