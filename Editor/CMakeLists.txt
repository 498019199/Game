include_directories(
    ${ZENGINE_ROOT_HEADER}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../Game/include)

set(GAME_APP_SRC_LIST
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GameApp.cpp)

if(WIN32)
    include_directories(
        ${DEMOENGINE_DIR}/3rd/dxsdk/dxsdk/Include
        ${DEMOENGINE_DIR}/core/plungins/render/sdl3/src)

    set(SRC_LIST
        ${CMAKE_CURRENT_SOURCE_DIR}/src/EditorConsolePanel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/EditorDialogBoxManager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/EditorGameViewPanel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/EditorHierarchyPanel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/EditorInspectorPanel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/EditorMainBarPanel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/EditorManager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/EditorManagerD3D11.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/EditorManagerSDL3.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/EditorProfilerPanel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/EditorProjectPanel.cpp)

    add_executable(ZENGINE_editor
        ${SRC_LIST}
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

    set_target_properties(ZENGINE_editor PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON)

    if(MSVC)
        target_compile_options(ZENGINE_editor PRIVATE /std:c++20)
    endif()

    add_dependencies(ZENGINE_editor ZENGINE_game ZENGINE_Render_SDL3)

    target_link_libraries(ZENGINE_editor
        PUBLIC
            ZENGINE_game
            ZENGINE_common
            ZENGINE_core
            ZENGINE_Render_D3D11
            ZENGINE_Render_SDL3
            ZENGINE_DevHelps
            ZENGINE_AudioEngine_XAudio
            ZENGINE_AudioDataSource_OggVorbis
            ZENGINE_InputEngine_InputSDL3
            imgui
        PRIVATE
            d3dcompiler)

    add_executable(ZENGINE_editor_tests EXCLUDE_FROM_ALL tests/EditorRegressionTests.cpp)
    target_link_libraries(ZENGINE_editor_tests PRIVATE ZENGINE_core ZENGINE_common gtest imgui)
    add_dependencies(ZENGINE_editor_tests ZENGINE_AudioEngine_XAudio ZENGINE_AudioDataSource_OggVorbis ZENGINE_InputEngine_InputSDL3)
endif()

add_executable(ZENGINE_game_app
    ${GAME_APP_SRC_LIST}
    ${CMAKE_CURRENT_SOURCE_DIR}/game_main.cpp)

set_target_properties(ZENGINE_game_app PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON)

if(MSVC)
    target_compile_options(ZENGINE_game_app PRIVATE /std:c++20)
endif()

# The shipped configuration selects SDL3 on every platform. Build the dynamically
# loaded renderer even when only the game executable is requested.
add_dependencies(ZENGINE_game_app ZENGINE_game ZENGINE_DevHelps ZENGINE_Render_SDL3)

target_link_libraries(ZENGINE_game_app
    PUBLIC
        ZENGINE_game
        ZENGINE_common
        ZENGINE_core)

if(WIN32)
    target_link_libraries(ZENGINE_game_app
        PUBLIC
            ZENGINE_Render_D3D11
            ZENGINE_AudioEngine_XAudio
            ZENGINE_AudioDataSource_OggVorbis
            ZENGINE_InputEngine_InputSDL3
        PRIVATE
            d3dcompiler)
else()
    target_link_libraries(ZENGINE_game_app
        PUBLIC
            ZENGINE_Render_SDL3
            ZENGINE_InputEngine_InputSDL3
            ZENGINE_AudioDataSource_OggVorbis)
endif()
