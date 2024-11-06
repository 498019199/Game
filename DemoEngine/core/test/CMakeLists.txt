include_directories(${PROJECT_SOURCE_DIR}/core/include
                    ${PROJECT_SOURCE_DIR}/common/include
                    ${PROJECT_SOURCE_DIR}/3rd/ini/ini/src
                    ${PROJECT_SOURCE_DIR}/3rd/assimp/assimp/include
                    ${PROJECT_SOURCE_DIR}/3rd/dxsdk/dxsdk/Include
                    ${PROJECT_SOURCE_DIR}/3rd/FreeImage/Source)

add_executable(test_app
    ${PROJECT_SOURCE_DIR}/core/test/test_main.cpp)

target_link_libraries(test_app PRIVATE 
    ${LIB_DIR_PATH}/common${DEMOENGINE_OUTPUT_SUFFIX}${CMAKE_DEBUG_POSTFIX}.lib
    ${LIB_DIR_PATH}/core${DEMOENGINE_OUTPUT_SUFFIX}${CMAKE_DEBUG_POSTFIX}.lib
    ${LIB_DIR_PATH}/math${DEMOENGINE_OUTPUT_SUFFIX}${CMAKE_DEBUG_POSTFIX}.lib
    ${LIB_DIR_PATH}/assimp${DEMOENGINE_OUTPUT_SUFFIX}${CMAKE_DEBUG_POSTFIX}.lib
    ${PROJECT_SOURCE_DIR}/3rd/dxsdk/lib/x64/d3dx9${3RD_DEBUG_POSTFIX}.lib
    ${PROJECT_SOURCE_DIR}/3rd/dxsdk/lib/x64/d3dx11${3RD_DEBUG_POSTFIX}.lib
    ${PROJECT_SOURCE_DIR}/3rd/dxsdk/lib/x64/dxgi.lib
    ${PROJECT_SOURCE_DIR}/3rd/dxsdk/lib/x64/DxErr.lib
    ${LIB_DIR_PATH}/LibPNG${CMAKE_DEBUG_POSTFIX}.lib
    ${LIB_DIR_PATH}/LibJPEG${CMAKE_DEBUG_POSTFIX}.lib)