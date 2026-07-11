include_directories(
    ${ZENGINE_ROOT_HEADER}
    ${DEMOENGINE_DIR}/3rd/dxsdk/dxsdk/Include
    ${DEMOENGINE_DIR}/3rd/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../Game/include)

set(SRC_LIST
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EditorConsolePanel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EditorDialogBoxManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EditorGameViewPanel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EditorHierarchyPanel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EditorInspectorPanel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EditorMainBarPanel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EditorManagerD3D11.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EditorProfilerPanel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EditorProjectPanel.cpp)

set(GAME_APP_SRC_LIST
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GameApp.cpp)

set(IMGUI_SRC_LIST
    ${DEMOENGINE_DIR}/3rd/imgui/imgui/imgui_demo.cpp
    ${DEMOENGINE_DIR}/3rd/imgui/imgui/imgui_draw.cpp
    ${DEMOENGINE_DIR}/3rd/imgui/imgui/imgui_impl_dx11.cpp
    ${DEMOENGINE_DIR}/3rd/imgui/imgui/imgui_impl_win32.cpp
    ${DEMOENGINE_DIR}/3rd/imgui/imgui/imgui_widgets.cpp
    ${DEMOENGINE_DIR}/3rd/imgui/imgui/imgui_tables.cpp
    ${DEMOENGINE_DIR}/3rd/imgui/imgui/imgui.cpp
)

add_executable(ZENGINE_editor
    ${SRC_LIST} ${IMGUI_SRC_LIST}
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

set_target_properties(ZENGINE_editor PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON)

if(MSVC)
    target_compile_options(ZENGINE_editor PRIVATE /std:c++20)
endif()

add_dependencies(ZENGINE_editor ZENGINE_game)

target_link_libraries(ZENGINE_editor
    PUBLIC
        ZENGINE_game
        ZENGINE_common
        ZENGINE_core
        ZENGINE_Render_D3D11
        ZENGINE_DevHelps
        ZENGINE_AudioEngine_XAudio
        ZENGINE_AudioDataSource_OggVorbis
        ZENGINE_InputEngine_Input
    PRIVATE
        d3dcompiler
)

add_executable(ZENGINE_game_app
    ${GAME_APP_SRC_LIST}
    ${CMAKE_CURRENT_SOURCE_DIR}/game_main.cpp)

set_target_properties(ZENGINE_game_app PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON)

if(MSVC)
    target_compile_options(ZENGINE_game_app PRIVATE /std:c++20)
endif()

add_dependencies(ZENGINE_game_app ZENGINE_game ZENGINE_DevHelps)

target_link_libraries(ZENGINE_game_app
    PUBLIC
        ZENGINE_game
        ZENGINE_common
        ZENGINE_core
        ZENGINE_Render_D3D11
        ZENGINE_AudioEngine_XAudio
        ZENGINE_AudioDataSource_OggVorbis
        ZENGINE_InputEngine_Input
    PRIVATE
        d3dcompiler
)
