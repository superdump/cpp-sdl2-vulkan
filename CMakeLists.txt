cmake_minimum_required(VERSION 3.1...3.13)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(scratchapixel
        VERSION 0.1.0
        DESCRIPTION "An implementation of scratchapixel.com"
        LANGUAGES CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${${PROJECT_NAME}_SOURCE_DIR}/cmake")

# Dependencies

# SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})

# scratchapixel

include_directories(include)

set(${PROJECT_NAME}_SOURCES
    src/${PROJECT_NAME}.cpp
)
add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SOURCES})
install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(FILES ${PROJECT_NAME}.h DESTINATION include)

set(example_SOURCES
    examples/main.cpp
)
add_executable(example ${example_SOURCES})
target_link_libraries(example ${PROJECT_NAME} ${SDL2_LIBRARY})
install(TARGETS example DESTINATION bin)
