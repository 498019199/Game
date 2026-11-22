find_package(Python3 COMPONENTS Interpreter REQUIRED)

set(TableGenOutput "${CMAKE_CURRENT_SOURCE_DIR}/src/tablegen/Tables.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/tablegen/Tables.cpp")
add_custom_command(OUTPUT ${TableGenOutput}
	COMMAND "${Python3_EXECUTABLE}" "TableGen.py"
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/tablegen/"
	DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/tablegen/TableGen.py"
	COMMENT "Generating tables..."
	VERBATIM)
add_custom_target(TableGen DEPENDS ${TableGenOutput})
set_target_properties(TableGen PROPERTIES
    FOLDER "ZENGINE"  # 将目标放在 ZENGINE 文件夹下
)

SOURCE_GROUP("App Layer\\Source Files" FILES ${APP_HEADER_FILES})
SOURCE_GROUP("App Layer\\Header Files" FILES ${APP_SOURCE_FILES})

set(APP_HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/base/App3D.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/base/Window.h
)

set(APP_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/app/App3D.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/app/Window.cpp
)

SOURCE_GROUP("Base\\Source Files" FILES ${BASE_SOURCE_FILES})
SOURCE_GROUP("Base\\Header Files" FILES ${BASE_HEADER_FILES})

set(BASE_HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/base/Context.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/base/DevHelper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/base/Input.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/base/InputFactory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/base/LZMACodec.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/base/Package.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/base/ResLoader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/base/RenderSettings.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tablegen/Tables.hpp
)

set(BASE_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/Context.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/DevHelper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/RenderableHelper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/ResLoader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base/TexCompression.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/pack/ArchiveExtractCallback.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pack/ArchiveOpenCallback.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pack/LZMACodec.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pack/Package.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pack/Streams.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/tablegen/Tables.cpp
)

SOURCE_GROUP("Base\\Source Files" FILES ${BASE_SOURCE_FILES})
SOURCE_GROUP("Base\\Header Files" FILES ${BASE_HEADER_FILES})

set(RENDER_HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/render/Camera.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/render/CascadedShadowLayer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/render/ElementFormat.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/render/Fence.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/render/GraphicsBuffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/render/Light.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/render/Mesh.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/render/Query.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/render/Renderable.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/render/EffectDesc.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/render/BuiltInEffects.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/render/ParticleEffect.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/render/ParticleRenderable.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/render/RenderableHelper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/render/RenderDeviceCaps.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/render/RenderEffect.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/render/RenderEngine.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/render/RenderFactory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/render/RenderLayout.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/render/RenderMaterial.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/render/RenderStateObject.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/render/RenderView.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/render/ShaderObject.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/render/SkyBox.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/render/TexCompression.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/render/TexCompressionBC.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/render/TransientBuffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/render/Texture.h
)

set(RENDER_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/Camera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/CascadedShadowLayer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/ElementFormat.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/FrameBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/GraphicsBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/Light.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/Mesh.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/Query.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/Renderable.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/EffectDesc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/BuiltInEffects.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/EffectDescRuntime.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/ParticleEffect.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/ParticleRenderable.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/RenderDeviceCaps.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/RenderEffect.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/RenderVariable.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/RenderEngine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/RenderFactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/RenderLayout.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/RenderMaterial.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/RenderStateObject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/RenderView.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/ShaderObject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/SkyBox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/TexCompressionBC.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/Texture.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/TransientBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/Viewport.cpp
)
SOURCE_GROUP("Rendering System\\Source Files" FILES ${RENDERING_SOURCE_FILES})
SOURCE_GROUP("Rendering System\\Header Files" FILES ${RENDERING_HEADER_FILES})


SET(AUDIO_HEADER_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/include/base/Audio.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/base/AudioDataSource.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/base/AudioFactory.h
)
SET(AUDIO_SOURCE_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/src/audio/AudioBuffer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/audio/AudioDataSource.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/audio/AudioEngine.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/audio/AudioFactory.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/audio/SoundBuffer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/audio/MusicBuffer.cpp
)
SOURCE_GROUP("Audio System\\Source Files" FILES ${AUDIO_SOURCE_FILES})
SOURCE_GROUP("Audio System\\Header Files" FILES ${AUDIO_HEADER_FILES})

set(WORLD_HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/world/World.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/world/CameraController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/world/SceneNode.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/world/SceneComponent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/world/ParticleSystem.h
)

set(WORLD_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/world/World.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/world/CameraController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/world/SceneNode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/world/SceneComponent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/world/ParticleSystem.cpp
)
SOURCE_GROUP("World\\Source Files" FILES ${AUDIO_SOURCE_FILES})
SOURCE_GROUP("World\\Header Files" FILES ${AUDIO_HEADER_FILES})

SOURCE_GROUP("Input\\Source Files" FILES ${INPUT_SOURCE_FILES})
set(INPUT_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/input/InputActionMap.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/input/InputDevice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/input/InputEngine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/input/InputFactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/input/Joystick.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/input/Keyboard.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/input/Mouse.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/input/Sensor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/input/Touch.cpp
)

set(UI_HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/base/UIManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/base/RmlUiRenderInterface.h
)
set(UI_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/UIInput.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/UIManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/RmlUiRenderInterface.cpp
)
SOURCE_GROUP("UI\\Source Files" FILES ${UI_SOURCE_FILES})
SOURCE_GROUP("UI\\Header Files" FILES ${UI_HEADER_FILES})

add_library(ZENGINE_core ${ZENGINE_PREFERRED_LIB_TYPE}
    ${APP_HEADER_FILES} ${APP_SOURCE_FILES}
    ${BASE_HEADER_FILES} ${BASE_SOURCE_FILES}
    ${WORLD_HEADER_FILES} ${WORLD_SOURCE_FILES}
    ${RENDER_HEADER_FILES} ${RENDER_SOURCE_FILES}
    ${AUDIO_HEADER_FILES} ${AUDIO_SOURCE_FILES}
    ${UI_HEADER_FILES} ${UI_SOURCE_FILES}
    ${INPUT_SOURCE_FILES}
)

add_dependencies(ZENGINE_core TableGen)

target_include_directories(ZENGINE_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_precompile_headers(ZENGINE_core
    PRIVATE
		include/base/ZEngine.h
)
set_target_properties(ZENGINE_core PROPERTIES
    OUTPUT_NAME ZENGINE_core${ZENGINE_OUTPUT_SUFFIX}
    FOLDER "ZENGINE"
)

target_compile_definitions(ZENGINE_core
    PRIVATE
        -DZENGINE_EXPORTS
)

target_link_libraries(ZENGINE_core
    PUBLIC
        NanoRtti
        ZENGINE_common
        ZENGINE_RmlUiHeaders
    PRIVATE
        7zxa
        LZMA
        rmlui
        rmlui_debugger
)

if(WIN32)
    target_sources(ZENGINE_core PRIVATE src/app/Win32Window.cpp)
    target_link_libraries(ZENGINE_core PRIVATE dxsdk d3d11 dxgi d3dcompiler user32)
endif()


if(ZENGINE_IS_DEV_PLATFORM)
	if(NOT (ZENGINE_ARCH_NAME STREQUAL "arm64"))
		#add_dependencies(ZENGINE_core D3DCompilerRedist)
	endif()
endif()
if(ZENGINE_PLATFORM_DARWIN)
    target_sources(ZENGINE_core PRIVATE src/app/CocoaWindow.mm)
    target_compile_options(ZENGINE_core PRIVATE $<$<COMPILE_LANGUAGE:OBJCXX>:-fobjc-arc>)
    target_link_libraries(ZENGINE_core PRIVATE "-framework AppKit")
endif()
