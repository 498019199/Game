option(ZENGINE_BUILD_PLUGIN_MSG_INPUT_INPUT_ENGINE "Build MsgInput input engine plugin" ON)
if(NOT ZENGINE_BUILD_PLUGIN_MSG_INPUT_INPUT_ENGINE)
	return()
endif()

IF(ZENGINE_PLATFORM_WINDOWS_DESKTOP)
	SET(ZENGINE_LibOVR_PATH "" CACHE STRING "Path of LibOVR")
	IF(ZENGINE_LibOVR_PATH)
		SET(ZENGINE_LIBOVR_FOUND TRUE)
	ENDIF()
ENDIF()

IF(ZENGINE_COMPILER_MSVC)
	SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /INCREMENTAL:NO")
ENDIF()

ADD_LIBRARY(ZENGINE_InputEngine_Input ${ZENGINE_PREFERRED_LIB_TYPE}
	src/MIJoystick.cpp
	src/MIKeyboard.cpp
	src/MIMouse.cpp
	src/MInput.hpp
	src/MInputEngine.cpp
	src/MInputFactory.cpp
	src/MISensor.cpp
	src/MITouch.cpp
	src/MIXInput.cpp
	src/MIOVR.cpp
)

if(ZENGINE_PLATFORM_WINDOWS AND ZENGINE_COMPILER_GCC)
	# GCC warnings emitted by preprocessor can't be suppressed with pragma GCC diagnostic ignored
	target_compile_options(ZENGINE_InputEngine_Input
		PRIVATE
			-Wno-unknown-pragmas
	)
endif()

IF(KLAYGE_LIBOVR_FOUND)
	target_compile_definitions(ZENGINE_InputEngine_Input
		PRIVATE
			-DZENGINE__HAVE_LIBOVR
	)

	target_include_directories(ZENGINE_InputEngine_Input
		PRIVATE
			${ZENGINE_LibOVR_PATH}/Include
	)
ENDIF()

SET_TARGET_PROPERTIES(ZENGINE_InputEngine_Input PROPERTIES
	OUTPUT_NAME ZENGINE_InputEngine_Input${ZENGINE_OUTPUT_SUFFIX}
	FOLDER "ZENGINE"
)
if(ZENGINE_PREFERRED_LIB_TYPE STREQUAL "SHARED")
	set_target_properties(ZENGINE_InputEngine_Input PROPERTIES
		CXX_VISIBILITY_PRESET hidden
		VISIBILITY_INLINES_HIDDEN ON
	)
endif()


target_precompile_headers(ZENGINE_InputEngine_Input
	PRIVATE
		"${PROJECT_SOURCE_DIR}/core/core/include/base/ZEngine.h"
)

target_link_libraries(ZENGINE_InputEngine_Input
	PRIVATE
		ZENGINE_common
		ZENGINE_core
)
if(ZENGINE_PLATFORM_WINDOWS)
	target_link_libraries(ZENGINE_InputEngine_Input
		PRIVATE
			dxsdk
			xinput
	)
endif()
if(ZENGINE_PLATFORM_WINDOWS_DESKTOP)
	target_link_libraries(ZENGINE_InputEngine_Input
		PRIVATE
			locationapi sensorsapi
	)
endif()
if(ZENGINE_LIBOVR_FOUND)
	target_link_libraries(ZENGINE_InputEngine_Input
		PRIVATE
			winmm
			debug ${ZENGINE_LibOVR_PATH}/Lib/${ZENGINE_VS_PLATFORM_NAME}/libovrd64.lib
			optimized ${ZENGINE_LibOVR_PATH}/Lib/${ZENGINE_VS_PLATFORM_NAME}/libovr64.lib
	)	
endif()