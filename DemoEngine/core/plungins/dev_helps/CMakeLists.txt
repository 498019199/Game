if (NOT ZENGINE_IS_DEV_PLATFORM)
	return()
endif()

add_library(ZENGINE_DevHelps ${ZENGINE_PREFERRED_LIB_TYPE}
    src/ImagePlane.h
    src/DevHelper.cpp
    src/ImagePlane.cpp
    src/TexConverter.cpp
    src/TexMetadata.cpp
)
target_include_directories(ZENGINE_DevHelps
	PUBLIC
		${ZENGINE_ROOT_HEADER}/include
        ${ZENGINE_ROOT_DIR}/core/plungins/dev_helps/include
)


if(ZENGINE_PREFERRED_LIB_TYPE STREQUAL "SHARED")
	set_target_properties(ZENGINE_DevHelps PROPERTIES
	CXX_VISIBILITY_PRESET hidden
	VISIBILITY_INLINES_HIDDEN ON
	)
endif()

target_precompile_headers(ZENGINE_DevHelps
	PRIVATE
		"${PROJECT_SOURCE_DIR}/core/core/include/base/ZEngine.h")


target_link_libraries(ZENGINE_DevHelps
        PRIVATE
            assimp
            FreeImage
            ZENGINE_core
    )

set_target_properties(ZENGINE_DevHelps PROPERTIES
	OUTPUT_NAME ZENGINE_DevHelps${ZENGINE_OUTPUT_SUFFIX}
	FOLDER "ZENGINE"
)
