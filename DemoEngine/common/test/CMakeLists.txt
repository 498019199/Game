include_directories(${PROJECT_SOURCE_DIR}/common/include)

add_executable(test_common
    ${PROJECT_SOURCE_DIR}/common/test/test_main.cpp)

target_link_libraries(test_common PRIVATE 
    ${LIB_DIR_PATH}/common${DEMOENGINE_OUTPUT_SUFFIX}${CMAKE_DEBUG_POSTFIX}.lib)