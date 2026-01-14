include_directories( 
    ${ZENGINE_ROOT_HEADER}
    ${PROJECT_SOURCE_DIR}/3rd/dxsdk/dxsdk/Include
    ${PROJECT_SOURCE_DIR}/3rd/imgui
    ${PROJECT_SOURCE_DIR}/editor/include)

set(SRC_LIST
    ${PROJECT_SOURCE_DIR}/Editor/src/EditorConsolePanel.cpp
    ${PROJECT_SOURCE_DIR}/Editor/src/EditorDialogBoxManager.cpp
    ${PROJECT_SOURCE_DIR}/Editor/src/EditorGameViewPanel.cpp
    ${PROJECT_SOURCE_DIR}/Editor/src/EditorHierarchyPanel.cpp
    ${PROJECT_SOURCE_DIR}/Editor/src/EditorInspectorPanel.cpp
    ${PROJECT_SOURCE_DIR}/Editor/src/EditorMainBarPanel.cpp
    ${PROJECT_SOURCE_DIR}/Editor/src/EditorManagerD3D11.cpp
    ${PROJECT_SOURCE_DIR}/Editor/src/EditorProjectPanel.cpp)

set(IMGUI_SRC_LIST
    ${PROJECT_SOURCE_DIR}/3rd/imgui/imgui/imgui_demo.cpp
    ${PROJECT_SOURCE_DIR}/3rd/imgui/imgui/imgui_draw.cpp
    ${PROJECT_SOURCE_DIR}/3rd/imgui/imgui/imgui_impl_dx11.cpp
    ${PROJECT_SOURCE_DIR}/3rd/imgui/imgui/imgui_impl_win32.cpp
    ${PROJECT_SOURCE_DIR}/3rd/imgui/imgui/imgui_widgets.cpp
    ${PROJECT_SOURCE_DIR}/3rd/imgui/imgui/imgui_tables.cpp
    ${PROJECT_SOURCE_DIR}/3rd/imgui/imgui/imgui.cpp
)

add_executable(ZENGINE_editor
    ${SRC_LIST} ${IMGUI_SRC_LIST}
    ${PROJECT_SOURCE_DIR}/Editor/main.cpp)

target_link_libraries(ZENGINE_editor 
    PUBLIC
        ZENGINE_common
        ZENGINE_core
        ZENGINE_Render_D3D11
        ZENGINE_DevHelps
        ZENGINE_AudioEngine_XAudio
        ZENGINE_AudioDataSource_OggVorbis
)
