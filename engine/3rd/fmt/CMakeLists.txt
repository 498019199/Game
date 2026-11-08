UpdateExternalLib("fmt" "https://github.com/fmtlib/fmt.git" "f5e54359df4c26b6230fc61d38aa294581393084")

set(CPACK_BINARY_NSIS OFF CACHE BOOL "" FORCE)
set(FMT_DOC OFF CACHE BOOL "" FORCE)
set(FMT_INSTALL OFF CACHE BOOL "" FORCE)
set(FMT_OS OFF CACHE BOOL "" FORCE)
set(FMT_TEST OFF CACHE BOOL "" FORCE)
set(FMT_DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX} CACHE STRING "" FORCE)

add_subdirectory(fmt)

# Apple Clang (incl. 21.x) still breaks fmt 10.x consteval format-string checks.
if(ZENGINE_PLATFORM_DARWIN OR ZENGINE_PLATFORM_IOS)
	target_compile_definitions(fmt PUBLIC FMT_CONSTEVAL=)
endif()

set_target_properties(fmt PROPERTIES
	FOLDER "3rd/fmt"
)
