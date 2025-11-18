# 音频解析封装
ADD_LIBRARY(ZENGINE_AudioDataSource_OggVorbis ${ZENGINE_PREFERRED_LIB_TYPE}
	src/OggVorbisSource.cpp
	src/OggVorbisSource.h
	src/OggVorbisSourceFactory.cpp
)

set_target_properties(ZENGINE_AudioDataSource_OggVorbis PROPERTIES
	OUTPUT_NAME ZENGINE_AudioDataSource_OggVorbis${ZENGINE_OUTPUT_SUFFIX}
	FOLDER "ZENGINE"
)

if(KLAYGE_PREFERRED_LIB_TYPE STREQUAL "SHARED")
	set_target_properties(ZENGINE_AudioDataSource_OggVorbis PROPERTIES
		CXX_VISIBILITY_PRESET hidden
		VISIBILITY_INLINES_HIDDEN ON
	)
endif()

target_precompile_headers(ZENGINE_AudioDataSource_OggVorbis
	PRIVATE
    "${PROJECT_SOURCE_DIR}/core/core/include/base/ZEngine.h")

target_link_libraries(ZENGINE_AudioDataSource_OggVorbis
	PRIVATE
        ZENGINE_core
		vorbisfile vorbis ogg
)

