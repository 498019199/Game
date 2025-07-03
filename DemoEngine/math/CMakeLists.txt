include_directories(${PROJECT_SOURCE_DIR}/math/include)

set(SRC_LIST
    ${PROJECT_SOURCE_DIR}/math/src/math.cpp
    ${PROJECT_SOURCE_DIR}/math/src/matrix.cpp
    ${PROJECT_SOURCE_DIR}/math/src/quaternion.cpp
    ${PROJECT_SOURCE_DIR}/math/src/rotator.cpp
    ${PROJECT_SOURCE_DIR}/math/src/color.cpp
    ${PROJECT_SOURCE_DIR}/math/src/half.cpp)

add_library(math STATIC ${SRC_LIST})
set_target_properties(math PROPERTIES
	DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
	OUTPUT_NAME math${ZENGINE_OUTPUT_SUFFIX}
	ARCHIVE_OUTPUT_NAME math${ZENGINE_OUTPUT_SUFFIX}
)
add_subdirectory(test)