include_directories(${PROJECT_SOURCE_DIR}/3rd/googletest/include
    ${PROJECT_SOURCE_DIR}/common/include
)

add_executable(test_math
    ${PROJECT_SOURCE_DIR}/common/test/test_math.cpp)

add_executable(test_common
    ${PROJECT_SOURCE_DIR}/common/test/test_main.cpp)

target_link_libraries(test_math PRIVATE 
    ${LIB_DIR_PATH}/common${ZENGINE_OUTPUT_SUFFIX}${CMAKE_DEBUG_POSTFIX}.lib
)  
set_target_properties(test_math PROPERTIES
    OUTPUT_NAME test_math${ZENGINE_OUTPUT_SUFFIX}
    FOLDER "test"
)

target_link_libraries(test_common PRIVATE 
    ${LIB_DIR_PATH}/common${ZENGINE_OUTPUT_SUFFIX}${CMAKE_DEBUG_POSTFIX}.lib
)  
set_target_properties(test_common PROPERTIES
    OUTPUT_NAME test_common${ZENGINE_OUTPUT_SUFFIX}
    FOLDER "test"
)
