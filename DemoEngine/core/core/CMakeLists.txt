SOURCE_GROUP("Base\\Source Files" FILES ${BASE_SOURCE_FILES})
SOURCE_GROUP("Base\\Header Files" FILES ${BASE_HEADER_FILES})

set(BASE_HEADER_FILES
    ${PROJECT_SOURCE_DIR}/core/core/include/Base/Context.h
    ${PROJECT_SOURCE_DIR}/core/core/include/Base/WinApp.h
    ${PROJECT_SOURCE_DIR}/core/core/include/Base/ResourceLoad.h
)

set(BASE_SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/core/core/src/app/WinApp.cpp
    ${PROJECT_SOURCE_DIR}/core/core/src/Context.cpp
    ${PROJECT_SOURCE_DIR}/core/core/src/RenderableHelper.cpp
    ${PROJECT_SOURCE_DIR}/core/core/src/ResourceLoad.cpp
    ${PROJECT_SOURCE_DIR}/core/core/src/TexCompression.cpp
)

SOURCE_GROUP("Base\\Source Files" FILES ${BASE_SOURCE_FILES})
SOURCE_GROUP("Base\\Header Files" FILES ${BASE_HEADER_FILES})
    
set(RENDER_HEADER_FILES
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
)

set(RENDER_SOURCE_FILES
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
)
SOURCE_GROUP("Rendering System\\Source Files" FILES ${RENDERING_SOURCE_FILES})
SOURCE_GROUP("Rendering System\\Header Files" FILES ${RENDERING_HEADER_FILES})

set(WORLD_HEADER_FILES
    ${PROJECT_SOURCE_DIR}/core/core/include/World/World.h
    ${PROJECT_SOURCE_DIR}/core/core/include/World/Control.h
    ${PROJECT_SOURCE_DIR}/core/core/include/World/SceneNode.h
)

set(WORLD_SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/core/core/src/World/World.cpp
    ${PROJECT_SOURCE_DIR}/core/core/src/World/Control.cpp
    ${PROJECT_SOURCE_DIR}/core/core/src/World/SceneNode.cpp
)
SOURCE_GROUP("World\\Source Files" FILES ${AUDIO_SOURCE_FILES})
SOURCE_GROUP("World\\Header Files" FILES ${AUDIO_HEADER_FILES})

add_library(ZENGINE_core ${ZENGINE_PREFERRED_LIB_TYPE}
    ${BASE_HEADER_FILES} ${BASE_SOURCE_FILES}
    ${WORLD_HEADER_FILES} ${WORLD_SOURCE_FILES}
    ${RENDER_HEADER_FILES} ${RENDER_SOURCE_FILES}
)

target_include_directories(ZENGINE_core
    PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}/include
        ${ZENGINE_ROOT_DIR}/3rd/dxsdk/dxsdk/include
        ${ZENGINE_ROOT_DIR}/common/include
        ${ZENGINE_ROOT_DIR}/core/core/include
)

# target_precompile_headers(ZENGINE_core
#     PUBLIC
# 		include/base/Context.h
# )

set_target_properties(ZENGINE_core PROPERTIES
    OUTPUT_NAME ZENGINE_core${ZENGINE_OUTPUT_SUFFIX}
    FOLDER "ZENGINE"
)

if(ZENGINE_IS_DEV_PLATFORM)
	if(NOT (ZENGINE_ARCH_NAME STREQUAL "arm64"))
		add_dependencies(ZENGINE_core D3DCompilerRedist)
	endif()
endif()

add_subdirectory(test)