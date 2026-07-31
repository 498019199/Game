add_library(ZENGINE_Render_SDL3 ${ZENGINE_PREFERRED_LIB_TYPE}
	src/SDL3Fence.cpp
	src/SDL3Fence.h
	src/SDL3FrameBuffer.cpp
	src/SDL3FrameBuffer.h
	src/SDL3GraphicsBuffer.cpp
	src/SDL3GraphicsBuffer.h
	src/SDL3Query.cpp
	src/SDL3Query.h
	src/SDL3RenderEngine.cpp
	src/SDL3RenderEngine.h
	src/SDL3RenderFactory.cpp
	src/SDL3RenderFactory.h
	src/SDL3RenderLayout.cpp
	src/SDL3RenderLayout.h
	src/SDL3RenderStateObject.cpp
	src/SDL3RenderStateObject.h
	src/SDL3RenderView.cpp
	src/SDL3RenderView.h
	src/SDL3RenderWindow.cpp
	src/SDL3RenderWindow.h
	src/SDL3ShaderObject.cpp
	src/SDL3ShaderObject.h
	src/SDL3Texture.cpp
	src/SDL3Texture.h
	src/SDL3Util.cpp
	src/SDL3Util.h
)

if(ZENGINE_PREFERRED_LIB_TYPE STREQUAL "SHARED")
	set_target_properties(ZENGINE_Render_SDL3 PROPERTIES
		CXX_VISIBILITY_PRESET hidden
		VISIBILITY_INLINES_HIDDEN ON
	)
endif()

target_precompile_headers(ZENGINE_Render_SDL3
	PRIVATE
		"${PROJECT_SOURCE_DIR}/core/core/include/base/ZEngine.h")

target_include_directories(ZENGINE_Render_SDL3
	PUBLIC
		${ZENGINE_ROOT_HEADER}
		${ZENGINE_ROOT_DIR}/core/include
)

target_link_libraries(ZENGINE_Render_SDL3
	PRIVATE
		ZENGINE_common
		ZENGINE_core
		SDL3::SDL3
)

if(ZENGINE_PLATFORM_WINDOWS)
	target_link_libraries(ZENGINE_Render_SDL3 PRIVATE d3dcompiler)
	# dxcapi.h from Windows SDK (um)
	if(DEFINED CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
		set(_ZENGINE_WINSDK_VER "${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}")
	else()
		set(_ZENGINE_WINSDK_VER "10.0.26100.0")
	endif()
	set(_ZENGINE_DXC_INCLUDE "$ENV{WindowsSdkDir}Include/${_ZENGINE_WINSDK_VER}/um")
	if(NOT EXISTS "${_ZENGINE_DXC_INCLUDE}/dxcapi.h")
		set(_ZENGINE_DXC_INCLUDE "C:/Program Files (x86)/Windows Kits/10/Include/${_ZENGINE_WINSDK_VER}/um")
	endif()
	if(EXISTS "${_ZENGINE_DXC_INCLUDE}/dxcapi.h")
		target_include_directories(ZENGINE_Render_SDL3 PRIVATE "${_ZENGINE_DXC_INCLUDE}")
	endif()

	# Ship DXC runtime next to the plugin so D3D12/DXIL compile works without PATH setup.
	set(_ZENGINE_DXC_REDIST "C:/Program Files (x86)/Windows Kits/10/Redist/D3D/x64")
	if(EXISTS "${_ZENGINE_DXC_REDIST}/dxcompiler.dll")
		add_custom_command(TARGET ZENGINE_Render_SDL3 POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
				"${_ZENGINE_DXC_REDIST}/dxcompiler.dll"
				"$<TARGET_FILE_DIR:ZENGINE_Render_SDL3>/dxcompiler.dll"
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
				"${_ZENGINE_DXC_REDIST}/dxil.dll"
				"$<TARGET_FILE_DIR:ZENGINE_Render_SDL3>/dxil.dll"
			COMMENT "Copy DXC runtime next to ZENGINE_Render_SDL3")
	endif()
endif()

set_target_properties(ZENGINE_Render_SDL3 PROPERTIES
	OUTPUT_NAME ZENGINE_Render_SDL3${ZENGINE_OUTPUT_SUFFIX}
	FOLDER "ZENGINE"
)

target_compile_definitions(ZENGINE_Render_SDL3
	PRIVATE
		-DZENGINE_EXPORTS
)
