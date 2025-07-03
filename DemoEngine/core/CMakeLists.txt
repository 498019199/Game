include_directories( 
    ${PROJECT_SOURCE_DIR}/Chapter9/core/include
    ${PROJECT_SOURCE_DIR}/math/include
    ${PROJECT_SOURCE_DIR}/common/include
    ${PROJECT_SOURCE_DIR}/3rd/dxsdk/dxsdk/Include
    ${PROJECT_SOURCE_DIR}/3rd/FreeImage/FreeImage/Source
    ${PROJECT_SOURCE_DIR}/3rd/imgui)

set(SRC_LIST
    ${PROJECT_SOURCE_DIR}/Chapter9/core/src/App/WinApp.cpp

    ${PROJECT_SOURCE_DIR}/Chapter9/core/src/D3D11/D3D11GraphicsBuffer.cpp
    ${PROJECT_SOURCE_DIR}/Chapter9/core/src/D3D11/D3D11RenderEngine.cpp
    ${PROJECT_SOURCE_DIR}/Chapter9/core/src/D3D11/D3D11RenderLayout.cpp 
    ${PROJECT_SOURCE_DIR}/Chapter9/core/src/D3D11/D3D11RenderStateObject.cpp
    ${PROJECT_SOURCE_DIR}/Chapter9/core/src/D3D11/D3D11RenderView.cpp
    ${PROJECT_SOURCE_DIR}/Chapter9/core/src/D3D11/D3D11ShaderObject.cpp
    ${PROJECT_SOURCE_DIR}/Chapter9/core/src/D3D11/D3D11Util.cpp 
    ${PROJECT_SOURCE_DIR}/Chapter9/core/src/D3D11/D3D11Texture.cpp
    ${PROJECT_SOURCE_DIR}/Chapter9/core/src/D3D11/D3D11Texture2D.cpp
    ${PROJECT_SOURCE_DIR}/Chapter9/core/src/D3D11/D3D11RenderFactory.cpp

    ${PROJECT_SOURCE_DIR}/Chapter9/core/src/DevHelper/ImagePlane.cpp

    ${PROJECT_SOURCE_DIR}/Chapter9/core/src/Render/Camera.cpp 
    ${PROJECT_SOURCE_DIR}/Chapter9/core/src/Render/GraphicsBuffer.cpp
    ${PROJECT_SOURCE_DIR}/Chapter9/core/src/Render/Light.cpp 
    ${PROJECT_SOURCE_DIR}/Chapter9/core/src/Render/Mesh.cpp 
    ${PROJECT_SOURCE_DIR}/Chapter9/core/src/Render/Renderable.cpp
    ${PROJECT_SOURCE_DIR}/Chapter9/core/src/Render/RenderEffect.cpp 
    ${PROJECT_SOURCE_DIR}/Chapter9/core/src/Render/RenderEngine.cpp 
    ${PROJECT_SOURCE_DIR}/Chapter9/core/src/Render/RenderFactory.cpp 
    ${PROJECT_SOURCE_DIR}/Chapter9/core/src/Render/RenderLayout.cpp
    ${PROJECT_SOURCE_DIR}/Chapter9/core/src/Render/RenderStateObject.cpp
    ${PROJECT_SOURCE_DIR}/Chapter9/core/src/Render/RenderView.cpp
    ${PROJECT_SOURCE_DIR}/Chapter9/core/src/Render/ShaderObject.cpp
    ${PROJECT_SOURCE_DIR}/Chapter9/core/src/Render/Texture.cpp

    ${PROJECT_SOURCE_DIR}/Chapter9/core/src/Scene/SceneNode.cpp
    ${PROJECT_SOURCE_DIR}/Chapter9/core/src/Scene/World.cpp


    ${PROJECT_SOURCE_DIR}/Chapter9/core/src/Context.cpp
    ${PROJECT_SOURCE_DIR}/Chapter9/core/src/Control.cpp
    ${PROJECT_SOURCE_DIR}/Chapter9/core/src/RenderableHelper.cpp
    ${PROJECT_SOURCE_DIR}/Chapter9/core/src/TexCompression.cpp)
    
add_executable(Chapter9
    ${SRC_LIST} 
    ${PROJECT_SOURCE_DIR}/Chapter9/main_tree_billboard.cpp)

target_link_libraries(Chapter9 PRIVATE 
    ${LIB_DIR_PATH}/math${ZENGINE_OUTPUT_SUFFIX}${CMAKE_DEBUG_POSTFIX}.lib
    ${LIB_DIR_PATH}/common${ZENGINE_OUTPUT_SUFFIX}${CMAKE_DEBUG_POSTFIX}.lib
    ${LIB_DIR_PATH}/imgui${ZENGINE_OUTPUT_SUFFIX}${CMAKE_DEBUG_POSTFIX}.lib
    #${LIB_DIR_PATH}/FreeImage${ZENGINE_OUTPUT_SUFFIX}${CMAKE_DEBUG_POSTFIX}.lib
    ${PROJECT_SOURCE_DIR}/3rd/dxsdk/dxsdk/Lib/x64/d3d11.lib
    ${PROJECT_SOURCE_DIR}/3rd/dxsdk/dxsdk/Lib/x64/d3dx11d.lib
    ${PROJECT_SOURCE_DIR}/3rd/dxsdk/dxsdk/Lib/x64/dxgi.lib
    ${PROJECT_SOURCE_DIR}/3rd/dxsdk/dxsdk/Lib/x64/dxguid.lib) 
