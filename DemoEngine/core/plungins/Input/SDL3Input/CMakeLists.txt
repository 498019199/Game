option(ZENGINE_BUILD_PLUGIN_SDL3_INPUT_ENGINE "Build SDL3 input engine plugin" ON)
if(NOT ZENGINE_BUILD_PLUGIN_SDL3_INPUT_ENGINE)
	return()
endif()

add_library(ZENGINE_InputEngine_InputSDL3 ${ZENGINE_PREFERRED_LIB_TYPE}
	src/SDL3IJoystick.cpp
	src/SDL3IKeyboard.cpp
	src/SDL3IMouse.cpp
	src/SDL3Input.hpp
	src/SDL3InputEngine.cpp
	src/SDL3InputFactory.cpp
)

if(ZENGINE_PREFERRED_LIB_TYPE STREQUAL "SHARED")
	set_target_properties(ZENGINE_InputEngine_InputSDL3 PROPERTIES
		CXX_VISIBILITY_PRESET hidden
		VISIBILITY_INLINES_HIDDEN ON
	)
endif()

target_precompile_headers(ZENGINE_InputEngine_InputSDL3
	PRIVATE
		"${PROJECT_SOURCE_DIR}/core/core/include/base/ZEngine.h"
)

target_include_directories(ZENGINE_InputEngine_InputSDL3
	PUBLIC
		${ZENGINE_ROOT_HEADER}
		${ZENGINE_ROOT_DIR}/core/include
)

target_link_libraries(ZENGINE_InputEngine_InputSDL3
	PRIVATE
		ZENGINE_common
		ZENGINE_core
		SDL3::SDL3
)

set_target_properties(ZENGINE_InputEngine_InputSDL3 PROPERTIES
	OUTPUT_NAME ZENGINE_InputEngine_InputSDL3${ZENGINE_OUTPUT_SUFFIX}
	FOLDER "ZENGINE"
)

target_compile_definitions(ZENGINE_InputEngine_InputSDL3
	PRIVATE
		-DZENGINE_EXPORTS
)
