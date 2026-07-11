if(ZENGINE_PLATFORM_WINDOWS)
	return()
endif()

add_library(ZENGINE_Render_SDL3 ${ZENGINE_PREFERRED_LIB_TYPE}
	src/SDL3Fence.cpp
	src/SDL3Fence.h
	src/SDL3FrameBuffer.cpp
	src/SDL3FrameBuffer.h
	src/SDL3Query.cpp
	src/SDL3Query.h
	src/SDL3RenderEngine.cpp
	src/SDL3RenderEngine.h
	src/SDL3RenderFactory.cpp
	src/SDL3RenderFactory.h
	src/SDL3RenderStateObject.cpp
	src/SDL3RenderStateObject.h
	src/SDL3RenderView.cpp
	src/SDL3RenderView.h
	src/SDL3RenderWindow.cpp
	src/SDL3RenderWindow.h
	src/SDL3ShaderObject.cpp
	src/SDL3ShaderObject.h
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

set_target_properties(ZENGINE_Render_SDL3 PROPERTIES
	OUTPUT_NAME ZENGINE_Render_SDL3${ZENGINE_OUTPUT_SUFFIX}
	FOLDER "ZENGINE"
)

target_compile_definitions(ZENGINE_Render_SDL3
	PRIVATE
		-DZENGINE_EXPORTS
)
