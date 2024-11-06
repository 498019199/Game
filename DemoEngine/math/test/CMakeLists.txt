include_directories(${PROJECT_SOURCE_DIR}/core/include
                    ${PROJECT_SOURCE_DIR}/common/include
                    ${PROJECT_SOURCE_DIR}/3rd/ini/ini/src
                    ${PROJECT_SOURCE_DIR}/3rd/assimp/assimp/include
                    ${PROJECT_SOURCE_DIR}/3rd/dxsdk/dxsdk/Include
                    ${PROJECT_SOURCE_DIR}/3rd/FreeImage/Source)

add_executable(test_math
    ${PROJECT_SOURCE_DIR}/math/test/main.cpp)

target_link_libraries(test_math PRIVATE 
    ${LIB_DIR_PATH}/math${DEMOENGINE_OUTPUT_SUFFIX}${CMAKE_DEBUG_POSTFIX}.lib)