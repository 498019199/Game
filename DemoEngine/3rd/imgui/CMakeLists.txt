include_directories(${PROJECT_SOURCE_DIR}/imgui/imgui)

SET(IMGUI_PROJECT_DIR "${PROJECT_3RD_DIR}/imgui")
set(SRC_LIST
    ${IMGUI_PROJECT_DIR}/imgui/imgui.cpp
    ${IMGUI_PROJECT_DIR}/imgui/imgui_demo.cpp
    ${IMGUI_PROJECT_DIR}/imgui/imgui_draw.cpp
    ${IMGUI_PROJECT_DIR}/imgui/imgui_tables.cpp
    ${IMGUI_PROJECT_DIR}/imgui/imgui_widgets.cpp
    ${IMGUI_PROJECT_DIR}/imgui/imgui_impl_dx11.cpp)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    list(APPEND SRC_LIST ${IMGUI_PROJECT_DIR}/imgui/imgui_impl_win32.cpp)
endif()

add_library(imgui STATIC ${SRC_LIST})
set_target_properties(imgui PROPERTIES 
    DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
    OUTPUT_NAME imgui${ZENGINE_OUTPUT_SUFFIX}
    ARCHIVE_OUTPUT_NAME imgui${ZENGINE_OUTPUT_SUFFIX})