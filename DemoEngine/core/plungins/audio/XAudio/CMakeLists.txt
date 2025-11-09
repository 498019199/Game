if (NOT ZENGINE_PLATFORM_WINDOWS)
	return()
endif()


ADD_LIBRARY(ZENGINE_AudioEngine_XAudio SHARED
	src/XAAudio.h
	src/XAAudioEngine.cpp
	src/XAAudioFactory.cpp
	src/XAMusicBuffer.cpp
	src/XASoundBuffer.cpp
)

if(ZENGINE_PREFERRED_LIB_TYPE STREQUAL "SHARED")
	set_target_properties(ZENGINE_AudioEngine_XAudio PROPERTIES
		CXX_VISIBILITY_PRESET hidden
		VISIBILITY_INLINES_HIDDEN ON
	)
endif()

target_precompile_headers(ZENGINE_AudioEngine_XAudio
	PRIVATE
		"${PROJECT_SOURCE_DIR}/core/core/include/base/ZEngine.h")

target_link_libraries(ZENGINE_AudioEngine_XAudio
	PRIVATE
		ZENGINE_core
		dxsdk
)

if(ZENGINE_COMPILER_GCC)
	target_link_libraries(ZENGINE_AudioEngine_XAudio
		PRIVATE
			xaudio2_8
	)
endif()

set_target_properties(ZENGINE_AudioEngine_XAudio PROPERTIES
	OUTPUT_NAME ZENGINE_AudioEngine_XAudio${ZENGINE_OUTPUT_SUFFIX}
	FOLDER "ZENGINE"
)
