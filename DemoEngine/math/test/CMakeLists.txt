include_directories(${PROJECT_SOURCE_DIR}/3rd/googletest/include)

add_executable(test_math
    ${PROJECT_SOURCE_DIR}/math/test/test_math.cpp)

target_link_libraries(test_math PRIVATE 
    ${LIB_DIR_PATH}/math${ZENGINE_OUTPUT_SUFFIX}${CMAKE_DEBUG_POSTFIX}.lib)