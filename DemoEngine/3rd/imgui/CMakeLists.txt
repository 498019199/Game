
include_directories(${PROJECT_SOURCE_DIR}/imgui/imgui)

# imgui_demo uses the Direct3D 11 + Win32 backends; skip on non-Windows.
if(NOT ZENGINE_PLATFORM_WINDOWS)
	return()
endif()

SET(IMGUI_PROJECT_DIR "${PROJECT_3RD_DIR}/imgui")
set(SRC_LIST
    ${IMGUI_PROJECT_DIR}/imgui/imgui.cpp
    ${IMGUI_PROJECT_DIR}/imgui/imgui_demo.cpp
    ${IMGUI_PROJECT_DIR}/imgui/imgui_draw.cpp
    ${IMGUI_PROJECT_DIR}/imgui/imgui_tables.cpp
    ${IMGUI_PROJECT_DIR}/imgui/imgui_widgets.cpp
    ${IMGUI_PROJECT_DIR}/imgui/imgui_impl_dx11.cpp
    ${IMGUI_PROJECT_DIR}/imgui/imgui_impl_win32.cpp
    ${IMGUI_PROJECT_DIR}/imgui/main_imgui.cpp)

add_executable(imgui_demo ${SRC_LIST})

set_target_properties(imgui_demo PROPERTIES
    OUTPUT_NAME imgui_demo${ZENGINE_OUTPUT_SUFFIX}
    FOLDER "test"
)
