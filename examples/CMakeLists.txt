set(example_SOURCES
    
)
add_executable(example main.cpp)
if(ipo_supported)
    set_target_properties(example PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_link_libraries(example PRIVATE ${PROJECT_NAME} ${PROJECT_DEPS})