set(COMMON_PROJECT_DIR "${ZENGINE_ROOT_DIR}/common")

set(COMMON_HEADER_FILES
    ${PROJECT_SOURCE_DIR}/common/include/common/com_ptr.h
    ${PROJECT_SOURCE_DIR}/common/include/common/Compiler.h
    ${PROJECT_SOURCE_DIR}/common/include/common/CustomizedStreamBuf.h
    ${PROJECT_SOURCE_DIR}/common/include/common/defer.h
    ${PROJECT_SOURCE_DIR}/common/include/common/DllLoader.h
    ${PROJECT_SOURCE_DIR}/common/include/common/ErrorHandling.h
    ${PROJECT_SOURCE_DIR}/common/include/common/Hash.h
    ${PROJECT_SOURCE_DIR}/common/include/common/instance.h
    ${PROJECT_SOURCE_DIR}/common/include/common/JsonDom.h
    ${PROJECT_SOURCE_DIR}/common/include/common/Log.h
    ${PROJECT_SOURCE_DIR}/common/include/common/macro.h
    ${PROJECT_SOURCE_DIR}/common/include/common/ResIdentifier.h
    ${PROJECT_SOURCE_DIR}/common/include/common/span.h
    ${PROJECT_SOURCE_DIR}/common/include/common/StringUtil.h
    ${PROJECT_SOURCE_DIR}/common/include/common/Timer.h
    ${PROJECT_SOURCE_DIR}/common/include/common/Util.h
    ${PROJECT_SOURCE_DIR}/common/include/common/Uuid.h
    ${PROJECT_SOURCE_DIR}/common/include/common/XMLDom.h
)

set(COMMON_SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/common/src/common/CustomizedStreamBuf.cpp
    ${PROJECT_SOURCE_DIR}/common/src/common/DllLoader.cpp
    ${PROJECT_SOURCE_DIR}/common/src/common/ErrorHandling.cpp
    ${PROJECT_SOURCE_DIR}/common/src/common/JsonDom.cpp
    ${PROJECT_SOURCE_DIR}/common/src/common/Log.cpp
    ${PROJECT_SOURCE_DIR}/common/src/common/Timer.cpp
    ${PROJECT_SOURCE_DIR}/common/src/common/Util.cpp
    ${PROJECT_SOURCE_DIR}/common/src/common/XMLDom.cpp
)

set(MATH_HEADER_FILES
    ${PROJECT_SOURCE_DIR}/common/include/math/AABBox.h
    ${PROJECT_SOURCE_DIR}/common/include/math/color.h
    ${PROJECT_SOURCE_DIR}/common/include/math/half.h
    ${PROJECT_SOURCE_DIR}/common/include/math/math_helper.h
    ${PROJECT_SOURCE_DIR}/common/include/math/math.h
    ${PROJECT_SOURCE_DIR}/common/include/math/matrix.h
    ${PROJECT_SOURCE_DIR}/common/include/math/quaternion.h
    ${PROJECT_SOURCE_DIR}/common/include/math/rotator.h
    ${PROJECT_SOURCE_DIR}/common/include/math/vectorxd.h
)

set(MATH_SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/common/src/math/AABBox.cpp
    ${PROJECT_SOURCE_DIR}/common/src/math/color.cpp
    ${PROJECT_SOURCE_DIR}/common/src/math/half.cpp
    ${PROJECT_SOURCE_DIR}/common/src/math/math.cpp
    ${PROJECT_SOURCE_DIR}/common/src/math/matrix.cpp
    ${PROJECT_SOURCE_DIR}/common/src/math/quaternion.cpp
    ${PROJECT_SOURCE_DIR}/common/src/math/rotator.cpp
)

source_group("Base\\Source Files" FILES ${COMMON_HEADER_FILES})
source_group("Base\\Header Files" FILES ${COMMON_SOURCE_FILES})
source_group("Math\\Source Files" FILES ${MATH_SOURCE_FILES})
source_group("Math\\Header Files" FILES ${MATH_HEADER_FILES})

add_library(ZENGINE_common ${ZENGINE_PREFERRED_LIB_TYPE}
	${COMMON_SOURCE_FILES} ${COMMON_HEADER_FILES}
	${MATH_SOURCE_FILES} ${MATH_HEADER_FILES}
)

target_link_libraries(ZENGINE_common
	PRIVATE
		rapidjson
		rapidxml
)

target_include_directories(ZENGINE_common 
	PUBLIC
		${ZENGINE_ROOT_HEADER}/include
		${COMMON_PROJECT_DIR}/include
)

set_target_properties(ZENGINE_common PROPERTIES
    OUTPUT_NAME ZENGINE_common${ZENGINE_OUTPUT_SUFFIX}
    FOLDER "ZENGINE"
)

target_precompile_headers(ZENGINE_common
	PRIVATE
		include/common/common.h
)


add_subdirectory(test)