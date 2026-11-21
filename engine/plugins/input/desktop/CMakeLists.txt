option(ZENGINE_BUILD_PLUGIN_DESKTOP_INPUT_ENGINE "Build Desktop input engine plugin" ON)
if(NOT ZENGINE_BUILD_PLUGIN_DESKTOP_INPUT_ENGINE)
	return()
endif()

add_library(ZENGINE_InputEngine_InputDesktop ${ZENGINE_PREFERRED_LIB_TYPE}
	src/DesktopIKeyboard.cpp
	src/DesktopIMouse.cpp
	src/DesktopInput.hpp
	src/DesktopInputEngine.cpp
	src/DesktopInputFactory.cpp
)

if(ZENGINE_PREFERRED_LIB_TYPE STREQUAL "SHARED")
	set_target_properties(ZENGINE_InputEngine_InputDesktop PROPERTIES
		CXX_VISIBILITY_PRESET hidden
		VISIBILITY_INLINES_HIDDEN ON
	)
endif()

target_precompile_headers(ZENGINE_InputEngine_InputDesktop
	PRIVATE
		<base/ZEngine.h>
)


target_link_libraries(ZENGINE_InputEngine_InputDesktop
	PRIVATE
		ZENGINE_common
		ZENGINE_core
)

set_target_properties(ZENGINE_InputEngine_InputDesktop PROPERTIES
	OUTPUT_NAME ZENGINE_InputEngine_InputDesktop${ZENGINE_OUTPUT_SUFFIX}
	FOLDER "ZENGINE"
)

target_compile_definitions(ZENGINE_InputEngine_InputDesktop
	PRIVATE
		-DZENGINE_EXPORTS
)

if(WIN32)
    target_sources(ZENGINE_InputEngine_InputDesktop PRIVATE src/WinGamepad.cpp)
    target_link_libraries(ZENGINE_InputEngine_InputDesktop PRIVATE xinput)
else()
    target_sources(ZENGINE_InputEngine_InputDesktop PRIVATE src/MacGamepad.mm)
    set_source_files_properties(src/MacGamepad.mm PROPERTIES SKIP_PRECOMPILE_HEADERS ON COMPILE_FLAGS "-fobjc-arc")
    target_link_libraries(ZENGINE_InputEngine_InputDesktop PRIVATE "-framework Foundation" "-framework GameController" "-framework CoreHaptics")
endif()
