SOURCE_GROUP("App Layer\\Source Files" FILES ${APP_HEADER_FILES})
SOURCE_GROUP("App Layer\\Header Files" FILES ${APP_SOURCE_FILES})

set(APP_HEADER_FILES
    ${PROJECT_SOURCE_DIR}/core/core/include/base/App3D.h
    ${PROJECT_SOURCE_DIR}/core/core/include/base/Window.h
)

set(APP_SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/core/core/src/app/App3D.cpp
    ${PROJECT_SOURCE_DIR}/core/core/src/app/Window.cpp
)

if(ZENGINE_PLATFORM_WINDOWS)
    set(APP_SOURCE_FILES ${APP_SOURCE_FILES}
    ${PROJECT_SOURCE_DIR}/core/core/src/app/WindowWin.cpp
)
elseif(KLAYGE_PLATFORM_ANDROID)
elseif(KLAYGE_PLATFORM_IOS)
endif()

SOURCE_GROUP("Base\\Source Files" FILES ${BASE_SOURCE_FILES})
SOURCE_GROUP("Base\\Header Files" FILES ${BASE_HEADER_FILES})

set(BASE_HEADER_FILES
    ${PROJECT_SOURCE_DIR}/core/core/include/base/Context.h
    ${PROJECT_SOURCE_DIR}/core/core/include/base/CpuInfo.h
    ${PROJECT_SOURCE_DIR}/core/core/include/base/LZMACodec.h
    ${PROJECT_SOURCE_DIR}/core/core/include/base/Package.h
    ${PROJECT_SOURCE_DIR}/core/core/include/base/ResLoader.h
    ${PROJECT_SOURCE_DIR}/core/core/include/base/Thread.h
    ${PROJECT_SOURCE_DIR}/core/core/include/base/RenderSettings.h
)

set(BASE_SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/core/core/src/base/Context.cpp
    ${PROJECT_SOURCE_DIR}/core/core/src/base/CpuInfo.cpp
    ${PROJECT_SOURCE_DIR}/core/core/src/base/RenderableHelper.cpp
    ${PROJECT_SOURCE_DIR}/core/core/src/base/ResLoader.cpp
    ${PROJECT_SOURCE_DIR}/core/core/src/base/TexCompression.cpp
    ${PROJECT_SOURCE_DIR}/core/core/src/base/Thread.cpp

    ${PROJECT_SOURCE_DIR}/core/core/src/pack/ArchiveExtractCallback.cpp
    ${PROJECT_SOURCE_DIR}/core/core/src/pack/ArchiveOpenCallback.cpp
    ${PROJECT_SOURCE_DIR}/core/core/src/pack/LZMACodec.cpp
    ${PROJECT_SOURCE_DIR}/core/core/src/pack/Package.cpp
    ${PROJECT_SOURCE_DIR}/core/core/src/pack/Streams.cpp
)

SOURCE_GROUP("Base\\Source Files" FILES ${BASE_SOURCE_FILES})
SOURCE_GROUP("Base\\Header Files" FILES ${BASE_HEADER_FILES})
    
set(RENDER_HEADER_FILES
    ${PROJECT_SOURCE_DIR}/core/core/include/render/Camera.h
    ${PROJECT_SOURCE_DIR}/core/core/include/render/ElementFormat.h
    ${PROJECT_SOURCE_DIR}/core/core/include/render/Fen.h
    ${PROJECT_SOURCE_DIR}/core/core/include/render/GraphicsBuffer.h
    ${PROJECT_SOURCE_DIR}/core/core/include/render/Light.h 
    ${PROJECT_SOURCE_DIR}/core/core/include/render/Mesh.h 
    ${PROJECT_SOURCE_DIR}/core/core/include/render/Renderable.h
    ${PROJECT_SOURCE_DIR}/core/core/include/render/RenderableHelper.h
    ${PROJECT_SOURCE_DIR}/core/core/include/render/RenderDeviceCaps.h
    ${PROJECT_SOURCE_DIR}/core/core/include/render/RenderEffect.h 
    ${PROJECT_SOURCE_DIR}/core/core/include/render/RenderEngine.h 
    ${PROJECT_SOURCE_DIR}/core/core/include/render/RenderFactory.h 
    ${PROJECT_SOURCE_DIR}/core/core/include/render/RenderLayout.h
    ${PROJECT_SOURCE_DIR}/core/core/include/render/RenderStateObject.h
    ${PROJECT_SOURCE_DIR}/core/core/include/render/RenderView.h
    ${PROJECT_SOURCE_DIR}/core/core/include/render/ShaderObject.h
    ${PROJECT_SOURCE_DIR}/core/core/include/render/TexCompression.h
    ${PROJECT_SOURCE_DIR}/core/core/include/render/Texture.h
)

set(RENDER_SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/core/core/src/render/Camera.cpp 
    ${PROJECT_SOURCE_DIR}/core/core/src/render/FrameBuffer.cpp
    ${PROJECT_SOURCE_DIR}/core/core/src/render/GraphicsBuffer.cpp
    ${PROJECT_SOURCE_DIR}/core/core/src/render/Light.cpp 
    ${PROJECT_SOURCE_DIR}/core/core/src/render/Mesh.cpp 
    ${PROJECT_SOURCE_DIR}/core/core/src/render/Renderable.cpp
    ${PROJECT_SOURCE_DIR}/core/core/src/render/RenderEffect.cpp 
    ${PROJECT_SOURCE_DIR}/core/core/src/render/RenderEngine.cpp 
    ${PROJECT_SOURCE_DIR}/core/core/src/render/RenderFactory.cpp 
    ${PROJECT_SOURCE_DIR}/core/core/src/render/RenderLayout.cpp
    ${PROJECT_SOURCE_DIR}/core/core/src/render/RenderStateObject.cpp
    ${PROJECT_SOURCE_DIR}/core/core/src/render/RenderView.cpp
    ${PROJECT_SOURCE_DIR}/core/core/src/render/ShaderObject.cpp
    ${PROJECT_SOURCE_DIR}/core/core/src/render/Texture.cpp
    ${PROJECT_SOURCE_DIR}/core/core/src/render/Viewport.cpp
)
SOURCE_GROUP("Rendering System\\Source Files" FILES ${RENDERING_SOURCE_FILES})
SOURCE_GROUP("Rendering System\\Header Files" FILES ${RENDERING_HEADER_FILES})

set(WORLD_HEADER_FILES
    ${PROJECT_SOURCE_DIR}/core/core/include/world/World.h
    ${PROJECT_SOURCE_DIR}/core/core/include/world/Control.h
    ${PROJECT_SOURCE_DIR}/core/core/include/world/SceneNode.h
)

set(WORLD_SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/core/core/src/world/World.cpp
    ${PROJECT_SOURCE_DIR}/core/core/src/world/Control.cpp
    ${PROJECT_SOURCE_DIR}/core/core/src/world/SceneNode.cpp
)
SOURCE_GROUP("World\\Source Files" FILES ${AUDIO_SOURCE_FILES})
SOURCE_GROUP("World\\Header Files" FILES ${AUDIO_HEADER_FILES})

add_library(ZENGINE_core ${ZENGINE_PREFERRED_LIB_TYPE}
    ${APP_HEADER_FILES} ${APP_SOURCE_FILES}
    ${BASE_HEADER_FILES} ${BASE_SOURCE_FILES}
    ${WORLD_HEADER_FILES} ${WORLD_SOURCE_FILES}
    ${RENDER_HEADER_FILES} ${RENDER_SOURCE_FILES}
)

target_include_directories(ZENGINE_core
    PUBLIC
        ${ZENGINE_ROOT_DIR}/common/include
        ${ZENGINE_ROOT_DIR}/core/core/include
        ${ZENGINE_ROOT_DIR}/3rd/dxsdk/dxsdk/include
        ${ZENGINE_ROOT_DIR}/3rd/7z/7z
)

target_precompile_headers(ZENGINE_core
    PUBLIC
		include/base/ZEngine.h
)
set_target_properties(ZENGINE_core PROPERTIES
    OUTPUT_NAME ZENGINE_core${ZENGINE_OUTPUT_SUFFIX}
    FOLDER "ZENGINE"
)

target_link_libraries(ZENGINE_core
    PUBLIC
        ZENGINE_common
    PRIVATE
        7zxa
        LZMA
)


if(ZENGINE_IS_DEV_PLATFORM)
	if(NOT (ZENGINE_ARCH_NAME STREQUAL "arm64"))
		#add_dependencies(ZENGINE_core D3DCompilerRedist)
	endif()
endif()