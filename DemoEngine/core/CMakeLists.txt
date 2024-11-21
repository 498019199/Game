include_directories(${PROJECT_SOURCE_DIR}/core/include
                    ${PROJECT_SOURCE_DIR}/common/include
                    ${PROJECT_SOURCE_DIR}/math/include
                    ${PROJECT_SOURCE_DIR}/3rd/ini/ini/src
                    ${PROJECT_SOURCE_DIR}/3rd/assimp/assimp/include
                    ${PROJECT_SOURCE_DIR}/3rd/dxsdk/dxsdk/Include
                    ${PROJECT_SOURCE_DIR}/3rd/FreeImage/Source)

set(SRCS_LIST   ${PROJECT_SOURCE_DIR}/core/src/IContext.cpp
########################################  Render Base  ##################################################
                        ${PROJECT_SOURCE_DIR}/core/src/render/IRenderDevice.cpp
                        ${PROJECT_SOURCE_DIR}/core/src/render/ElementFormat.cpp
                        ${PROJECT_SOURCE_DIR}/core/src/render/IRenderMaterial.cpp
                        ${PROJECT_SOURCE_DIR}/core/src/render/IRenderModel.cpp
                        ${PROJECT_SOURCE_DIR}/core/src/render/pack/MeshLoader.cpp
                        ${PROJECT_SOURCE_DIR}/core/src/render/pack/TexMetadata.cpp
###########################################  OpenGL  ####################################################
                )

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    list(APPEND SRCS_LIST   ${PROJECT_SOURCE_DIR}/core/src/app/windows/WindowApp.cpp
###########################################  D3D9  ######################################################
                            ${PROJECT_SOURCE_DIR}/core/src/render/directX9/D3D9Adapter.cpp
                            ${PROJECT_SOURCE_DIR}/core/src/render/directX9/D3D9AdapterList.cpp
                            ${PROJECT_SOURCE_DIR}/core/src/render/directX9/D3D9RenderDevice.cpp
                            ${PROJECT_SOURCE_DIR}/core/src/render/directX9/D3D9RenderShaderObject.cpp
                            ${PROJECT_SOURCE_DIR}/core/src/render/directX9/D3D9Texture.cpp
                            ${PROJECT_SOURCE_DIR}/core/src/render/directX9/D3D9Texture2D.cpp
                            ${PROJECT_SOURCE_DIR}/core/src/render/directX9/D3D9Texture3D.cpp
                            ${PROJECT_SOURCE_DIR}/core/src/render/directX9/D3D9TextureCube.cpp
###########################################  D3D11  #####################################################
                            ${PROJECT_SOURCE_DIR}/core/src/render/directX11/D3D11Adapter.cpp
                            ${PROJECT_SOURCE_DIR}/core/src/render/directX11/D3D11AdapterList.cpp
                            ${PROJECT_SOURCE_DIR}/core/src/render/directX11/D3D11RenderDevice.cpp
                            ${PROJECT_SOURCE_DIR}/core/src/render/directX11/D3D11RenderShaderObject.cpp
                            ${PROJECT_SOURCE_DIR}/core/src/render/directX11/D3D11Texture.cpp
                            ${PROJECT_SOURCE_DIR}/core/src/render/directX11/D3D11Texture2D.cpp
                            ${PROJECT_SOURCE_DIR}/core/src/render/directX11/D3D11Texture3D.cpp
                            ${PROJECT_SOURCE_DIR}/core/src/render/directX11/D3D11TextureCube.cpp
                            ${PROJECT_SOURCE_DIR}/core/src/render/directX11/D3D11RenderWindow.cpp
                            ${PROJECT_SOURCE_DIR}/core/src/render/directX11/D3D11Util.cpp
###########################################  D3D12  #####################################################
        )
endif()

add_library(core STATIC ${SRCS_LIST}
        ${LIB_DIR_PATH}/common${DEMOENGINE_OUTPUT_SUFFIX}${CMAKE_DEBUG_POSTFIX}.lib
        ${LIB_DIR_PATH}/math${DEMOENGINE_OUTPUT_SUFFIX}${CMAKE_DEBUG_POSTFIX}.lib
        ${LIB_DIR_PATH}/assimp${DEMOENGINE_OUTPUT_SUFFIX}${CMAKE_DEBUG_POSTFIX}.lib
        ${LIB_DIR_PATH}/LibPNG${CMAKE_DEBUG_POSTFIX}.lib
        ${LIB_DIR_PATH}/LibJPEG${CMAKE_DEBUG_POSTFIX}.lib)
set_target_properties(core PROPERTIES
	DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
	OUTPUT_NAME core${DEMOENGINE_OUTPUT_SUFFIX})

add_subdirectory(test)