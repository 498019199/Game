SET(SOURCE_FILES
	${PROJECT_SOURCE_DIR}/tests/src/CTHashTest.cpp
	${PROJECT_SOURCE_DIR}/tests/src/ResLoaderTest.cpp
	${PROJECT_SOURCE_DIR}/tests/src/StringUtilTest.cpp
	${PROJECT_SOURCE_DIR}/tests/src/test.cpp
	${PROJECT_SOURCE_DIR}/tests/src/test_math.cpp
)

SET(HEADER_FILES
	${PROJECT_SOURCE_DIR}/tests/src/test.h
)

add_executable(Tests ${SOURCE_FILES} ${HEADER_FILES})

SET_TARGET_PROPERTIES(Tests PROPERTIES
	DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
	CXX_VISIBILITY_PRESET hidden
	VISIBILITY_INLINES_HIDDEN ON
	OUTPUT_NAME Tests${ZENGINE_OUTPUT_SUFFIX}
	FOLDER "Tests"
)

target_link_libraries(Tests
	PRIVATE
        ZENGINE_common
        ZENGINE_core
		gtest
)

target_compile_definitions(Tests PRIVATE ZENGINE_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/bin")
target_precompile_headers(Tests PRIVATE <base/ZEngine.h>)
