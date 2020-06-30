set(HEADER_LIST ${project_SOURCE_DIR}/include/${PROJECT_NAME}/${PROJECT_NAME}.h)

add_library(${PROJECT_NAME} ${PROJECT_NAME}.cpp ${HEADER_LIST})
if(ipo_supported)
    set_target_properties(${PROJECT_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_include_directories(${PROJECT_NAME} PUBLIC ../include)
target_link_libraries(${PROJECT_NAME} ${PROJECT_DEPS})

source_group(TREE "${PROJECT_SOURCE_DIR}/include" PREFIX "Header Files" FILES ${HEADER_LIST})