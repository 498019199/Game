include_directories(${PROJECT_SOURCE_DIR}/3rd/googletest/include
    ${PROJECT_SOURCE_DIR}/common/include
    ${PROJECT_SOURCE_DIR}/core/core/include
)

add_executable(test_render
    ${PROJECT_SOURCE_DIR}/common/test/test_main.cpp)

target_link_libraries(test_render PRIVATE 
    ${LIB_DIR_PATH}/common${ZENGINE_OUTPUT_SUFFIX}${CMAKE_DEBUG_POSTFIX}.lib
)  
set_target_properties(test_render PROPERTIES
    OUTPUT_NAME test_render${ZENGINE_OUTPUT_SUFFIX}
    FOLDER "test"
)
