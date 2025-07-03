include_directories( 
    ${PROJECT_SOURCE_DIR}/core/include
    ${PROJECT_SOURCE_DIR}/math/include
    ${PROJECT_SOURCE_DIR}/common/include
    ${PROJECT_SOURCE_DIR}/3rd/dxsdk/dxsdk/Include
    #${PROJECT_SOURCE_DIR}/3rd/FreeImage/FreeImage/Source
    ${PROJECT_SOURCE_DIR}/3rd/imgui)

set(SRC_LIST
    ${PROJECT_SOURCE_DIR}/core/src/app/windows/WinApp.cpp

    ${PROJECT_SOURCE_DIR}/core/src/d3d11/D3D11GraphicsBuffer.cpp
    ${PROJECT_SOURCE_DIR}/core/src/d3d11/D3D11RenderEngine.cpp
    ${PROJECT_SOURCE_DIR}/core/src/d3d11/D3D11RenderLayout.cpp 
    ${PROJECT_SOURCE_DIR}/core/src/d3d11/D3D11RenderStateObject.cpp
    ${PROJECT_SOURCE_DIR}/core/src/d3d11/D3D11RenderView.cpp
    ${PROJECT_SOURCE_DIR}/core/src/d3d11/D3D11ShaderObject.cpp
    ${PROJECT_SOURCE_DIR}/core/src/d3d11/D3D11Util.cpp 
    ${PROJECT_SOURCE_DIR}/core/src/d3d11/D3D11Texture.cpp
    ${PROJECT_SOURCE_DIR}/core/src/d3d11/D3D11Texture2D.cpp
    ${PROJECT_SOURCE_DIR}/core/src/d3d11/D3D11RenderFactory.cpp

    ${PROJECT_SOURCE_DIR}/core/src/dev_helper/ImagePlane.cpp

    ${PROJECT_SOURCE_DIR}/core/src/render/Camera.cpp 
    ${PROJECT_SOURCE_DIR}/core/src/render/GraphicsBuffer.cpp
    ${PROJECT_SOURCE_DIR}/core/src/render/Light.cpp 
    ${PROJECT_SOURCE_DIR}/core/src/render/Mesh.cpp 
    ${PROJECT_SOURCE_DIR}/core/src/render/Renderable.cpp
    ${PROJECT_SOURCE_DIR}/core/src/render/RenderEffect.cpp 
    ${PROJECT_SOURCE_DIR}/core/src/render/RenderEngine.cpp 
    ${PROJECT_SOURCE_DIR}/core/src/render/RenderFactory.cpp 
    ${PROJECT_SOURCE_DIR}/core/src/render/RenderLayout.cpp
    ${PROJECT_SOURCE_DIR}/core/src/render/RenderStateObject.cpp
    ${PROJECT_SOURCE_DIR}/core/src/render/RenderView.cpp
    ${PROJECT_SOURCE_DIR}/core/src/render/ShaderObject.cpp
    ${PROJECT_SOURCE_DIR}/core/src/render/Texture.cpp

    ${PROJECT_SOURCE_DIR}/core/src/scene/SceneNode.cpp
    ${PROJECT_SOURCE_DIR}/core/src/scene/World.cpp

    ${PROJECT_SOURCE_DIR}/core/src/Context.cpp
    ${PROJECT_SOURCE_DIR}/core/src/Control.cpp
    ${PROJECT_SOURCE_DIR}/core/src/RenderableHelper.cpp
    ${PROJECT_SOURCE_DIR}/core/src/TexCompression.cpp)

target_link_libraries(d3d11_render PRIVATE 
     ${LIB_DIR_PATH}/math${ZENGINE_OUTPUT_SUFFIX}${CMAKE_DEBUG_POSTFIX}.lib
     ${LIB_DIR_PATH}/common${ZENGINE_OUTPUT_SUFFIX}${CMAKE_DEBUG_POSTFIX}.lib
     ${LIB_DIR_PATH}/imgui${ZENGINE_OUTPUT_SUFFIX}${CMAKE_DEBUG_POSTFIX}.lib)
#     #${LIB_DIR_PATH}/FreeImage${ZENGINE_OUTPUT_SUFFIX}${CMAKE_DEBUG_POSTFIX}.lib
#     ${PROJECT_SOURCE_DIR}/3rd/dxsdk/dxsdk/Lib/x64/d3d11.lib
#     ${PROJECT_SOURCE_DIR}/3rd/dxsdk/dxsdk/Lib/x64/d3dx11d.lib
#     ${PROJECT_SOURCE_DIR}/3rd/dxsdk/dxsdk/Lib/x64/dxgi.lib
#     ${PROJECT_SOURCE_DIR}/3rd/dxsdk/dxsdk/Lib/x64/dxguid.lib) 

add_subdirectory(test)