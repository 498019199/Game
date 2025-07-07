include_directories( 
    ${PROJECT_SOURCE_DIR}/core/core/include
    ${PROJECT_SOURCE_DIR}/math/include
    ${PROJECT_SOURCE_DIR}/common/include)

    
SOURCE_GROUP("Base\\Source Files" FILES ${BASE_SOURCE_FILES})
SOURCE_GROUP("Base\\Header Files" FILES ${BASE_HEADER_FILES})

set(BASE_HEADER_FILES
    ${PROJECT_SOURCE_DIR}/core/core/include/Base/common.h
    ${PROJECT_SOURCE_DIR}/core/core/include/Base/Context.h
    ${PROJECT_SOURCE_DIR}/core/core/include/Base/WinApp.h
    ${PROJECT_SOURCE_DIR}/core/core/include/Render/Adapter.h
    ${PROJECT_SOURCE_DIR}/core/core/include/Render/AdapterList.h
    ${PROJECT_SOURCE_DIR}/core/core/include/Render/Camera.h
    ${PROJECT_SOURCE_DIR}/core/core/include/Render/ElementFormat.h
    ${PROJECT_SOURCE_DIR}/core/core/include/Render/Fen.h
    ${PROJECT_SOURCE_DIR}/core/core/include/Render/GraphicsBuffer.h
    ${PROJECT_SOURCE_DIR}/core/core/include/Render/Light.h 
    ${PROJECT_SOURCE_DIR}/core/core/include/Render/Mesh.h 
    ${PROJECT_SOURCE_DIR}/core/core/include/Render/Renderable.h
    ${PROJECT_SOURCE_DIR}/core/core/include/Render/RenderableHelper.h
    ${PROJECT_SOURCE_DIR}/core/core/include/Render/RenderDeviceCaps.h
    ${PROJECT_SOURCE_DIR}/core/core/include/Render/RenderEffect.h 
    ${PROJECT_SOURCE_DIR}/core/core/include/Render/RenderEngine.h 
    ${PROJECT_SOURCE_DIR}/core/core/include/Render/RenderFactory.h 
    ${PROJECT_SOURCE_DIR}/core/core/include/Render/RenderLayout.h
    ${PROJECT_SOURCE_DIR}/core/core/include/Render/RenderStateObject.h
    ${PROJECT_SOURCE_DIR}/core/core/include/Render/RenderView.h
    ${PROJECT_SOURCE_DIR}/core/core/include/Render/ShaderObject.h
    ${PROJECT_SOURCE_DIR}/core/core/include/Render/TexCompression.h
    ${PROJECT_SOURCE_DIR}/core/core/include/Render/Texture.h
    ${PROJECT_SOURCE_DIR}/core/core/include/World/Control.h
    ${PROJECT_SOURCE_DIR}/core/core/include/World/SceneNode.h
    ${PROJECT_SOURCE_DIR}/core/core/include/World/World.h)

set(BASE_SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/core/core/src/App/WinApp.cpp
    ${PROJECT_SOURCE_DIR}/core/core/src/Render/Camera.cpp 
    ${PROJECT_SOURCE_DIR}/core/core/src/Render/GraphicsBuffer.cpp
    ${PROJECT_SOURCE_DIR}/core/core/src/Render/Light.cpp 
    ${PROJECT_SOURCE_DIR}/core/core/src/Render/Mesh.cpp 
    ${PROJECT_SOURCE_DIR}/core/core/src/Render/Renderable.cpp
    ${PROJECT_SOURCE_DIR}/core/core/src/Render/RenderEffect.cpp 
    ${PROJECT_SOURCE_DIR}/core/core/src/Render/RenderEngine.cpp 
    ${PROJECT_SOURCE_DIR}/core/core/src/Render/RenderFactory.cpp 
    ${PROJECT_SOURCE_DIR}/core/core/src/Render/RenderLayout.cpp
    ${PROJECT_SOURCE_DIR}/core/core/src/Render/RenderStateObject.cpp
    ${PROJECT_SOURCE_DIR}/core/core/src/Render/RenderView.cpp
    ${PROJECT_SOURCE_DIR}/core/core/src/Render/ShaderObject.cpp
    ${PROJECT_SOURCE_DIR}/core/core/src/Render/Texture.cpp
    ${PROJECT_SOURCE_DIR}/core/core/src/World/SceneNode.cpp
    ${PROJECT_SOURCE_DIR}/core/core/src/World/World.cpp
    ${PROJECT_SOURCE_DIR}/core/core/src/World/Control.cpp
    ${PROJECT_SOURCE_DIR}/core/core/src/Context.cpp
    ${PROJECT_SOURCE_DIR}/core/core/src/RenderableHelper.cpp
    ${PROJECT_SOURCE_DIR}/core/core/src/TexCompression.cpp)
    
add_library(DemoEngine_Core 
    ${BASE_SOURCE_FILES} ${BASE_HEADER_FILES})