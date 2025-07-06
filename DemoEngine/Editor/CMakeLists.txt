include_directories( 
    ${PROJECT_SOURCE_DIR}/core/include
    ${PROJECT_SOURCE_DIR}/math/include
    ${PROJECT_SOURCE_DIR}/common/include
    ${PROJECT_SOURCE_DIR}/3rd/dxsdk/dxsdk/Include
    ${PROJECT_SOURCE_DIR}/3rd/imgui
    ${PROJECT_SOURCE_DIR}/Editor/include)

set(SRC_LIST
    ${PROJECT_SOURCE_DIR}/Editor/src/EditorConsolePanel.cpp
    ${PROJECT_SOURCE_DIR}/Editor/src/EditorHierarchyPanel.cpp
    ${PROJECT_SOURCE_DIR}/Editor/src/EditorInspectorPanel.cpp
    ${PROJECT_SOURCE_DIR}/Editor/src/EditorMainBarPanel.cpp
    ${PROJECT_SOURCE_DIR}/Editor/src/EditorManagerD3D11.cpp
    ${PROJECT_SOURCE_DIR}/Editor/src/EditorProjectPanel.cpp)

add_executable(ZEditor
    ${SRC_LIST}
    ${PROJECT_SOURCE_DIR}/Editor/main.cpp)
