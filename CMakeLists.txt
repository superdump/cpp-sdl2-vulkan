cmake_minimum_required(VERSION 3.1...3.13)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(project
        VERSION 0.1.0
        DESCRIPTION "A clean SDL2 Vulkan project"
        LANGUAGES CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${${PROJECT_NAME}_SOURCE_DIR}/cmake")

# C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Interprocedural Optimization
include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported)

# Dependencies

# SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})

# Vulkan
find_package(vulkan REQUIRED)

set(PROJECT_DEPS ${SDL2_LIBRARY} Vulkan::Vulkan)


# project

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
