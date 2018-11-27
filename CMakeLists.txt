cmake_minimum_required(VERSION 3.0.0)
project(scratchapixel VERSION 0.1.0)

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
target_link_libraries(example ${PROJECT_NAME})
install(TARGETS example DESTINATION bin)
