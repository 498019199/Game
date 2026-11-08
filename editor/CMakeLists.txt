# Keep the editor widget independent of engine third-party configuration.
add_library(editor_imguizmo STATIC third_party/ImGuizmo/ImGuizmo.cpp)
target_link_libraries(editor_imguizmo PUBLIC imgui)
target_include_directories(editor_imguizmo PUBLIC third_party/ImGuizmo)
set_target_properties(editor_imguizmo PROPERTIES FOLDER "editor/3rd")

set(GAME_APP_SRC_LIST
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GameApp.cpp)

set(SRC_LIST
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EditorSetting.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EditorMaterialService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EditorMaterialPreview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EditorConsolePanel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EditorDialogBoxManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EditorGameViewPanel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EditorSceneGizmos.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EditorHierarchyPanel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EditorInspectorPanel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EditorMainBarPanel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EditorManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EditorManagerSDL3.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EditorProfilerPanel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EditorProjectPanel.cpp)

if(WIN32 OR ZENGINE_PLATFORM_DARWIN)
    list(APPEND SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/tests/EditorResizeSmoke.cpp)
endif()
if(WIN32)
    list(APPEND SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/src/EditorManagerD3D11.cpp)
endif()

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
    PRIVATE
        ZENGINE_game
        ZENGINE_common
        ZENGINE_core
        ZENGINE_Render_SDL3
        ZENGINE_DevHelps
        ZENGINE_AudioDataSource_OggVorbis
        ZENGINE_InputEngine_InputSDL3
        imgui
        editor_imguizmo
        SDL3::SDL3
        ZENGINE_RmlUiHeaders)

if(WIN32)
    target_link_libraries(ZENGINE_editor PRIVATE ZENGINE_Render_D3D11 ZENGINE_AudioEngine_XAudio d3dcompiler)
endif()

add_executable(ZENGINE_editor_tests EXCLUDE_FROM_ALL
    tests/EditorGizmoTests.cpp tests/EditorRegressionTests.cpp tests/MaterialSerializationTests.cpp tests/SDL3AudioTests.cpp
    tests/EditorMaterialDocumentTests.cpp tests/SceneMaterialTests.cpp tests/ParticleSystemTests.cpp tests/SceneParticleTests.cpp
    src/EditorMaterialService.cpp src/EditorSetting.cpp)
target_link_libraries(ZENGINE_editor_tests PRIVATE ZENGINE_game ZENGINE_core ZENGINE_common gtest imgui editor_imguizmo)
add_dependencies(ZENGINE_editor_tests ZENGINE_AudioDataSource_OggVorbis ZENGINE_InputEngine_InputSDL3)
if(WIN32)
    add_dependencies(ZENGINE_editor_tests ZENGINE_AudioEngine_XAudio)
endif()

if(WIN32 OR ZENGINE_PLATFORM_DARWIN)
    add_executable(ZENGINE_particle_gpu_tests EXCLUDE_FROM_ALL tests/ParticleGpuTests.cpp)
    target_link_libraries(ZENGINE_particle_gpu_tests PRIVATE ZENGINE_core ZENGINE_common ZENGINE_Render_SDL3 SDL3::SDL3)
    if(WIN32)
        target_link_libraries(ZENGINE_particle_gpu_tests PRIVATE ZENGINE_Render_D3D11)
    endif()
    set_target_properties(ZENGINE_particle_gpu_tests PROPERTIES
        CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON
        OUTPUT_NAME ZENGINE_particle_gpu_tests${ZENGINE_OUTPUT_SUFFIX}
        FOLDER "test")

    add_executable(ZENGINE_material_gpu_tests EXCLUDE_FROM_ALL tests/MaterialGpuTests.cpp)
    target_link_libraries(ZENGINE_material_gpu_tests PRIVATE ZENGINE_game ZENGINE_core ZENGINE_common ZENGINE_Render_SDL3 SDL3::SDL3)
    if(WIN32)
        target_link_libraries(ZENGINE_material_gpu_tests PRIVATE ZENGINE_Render_D3D11 dbghelp)
    endif()
    add_dependencies(ZENGINE_material_gpu_tests ZENGINE_DevHelps)
    set_target_properties(ZENGINE_material_gpu_tests PROPERTIES
        OUTPUT_NAME ZENGINE_material_gpu_tests${ZENGINE_OUTPUT_SUFFIX}
        FOLDER "test"
    )
endif()

set_target_properties(ZENGINE_editor_tests PROPERTIES
    OUTPUT_NAME ZENGINE_editor_tests${ZENGINE_OUTPUT_SUFFIX}
    FOLDER "test")

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
    PRIVATE
        ZENGINE_game
        ZENGINE_common
        ZENGINE_core)

if(WIN32)
    target_link_libraries(ZENGINE_game_app
        PRIVATE
            ZENGINE_Render_D3D11
            ZENGINE_AudioEngine_XAudio
            ZENGINE_AudioDataSource_OggVorbis
            ZENGINE_InputEngine_InputSDL3
        PRIVATE
            d3dcompiler)
else()
    target_link_libraries(ZENGINE_game_app
        PRIVATE
            ZENGINE_Render_SDL3
            ZENGINE_InputEngine_InputSDL3
            ZENGINE_AudioDataSource_OggVorbis)
endif()

foreach(app_target IN ITEMS ZENGINE_game_app ZENGINE_editor ZENGINE_editor_tests ZENGINE_material_gpu_tests ZENGINE_particle_gpu_tests)
    if(TARGET ${app_target})
        add_dependencies(${app_target} ZENGINE_AudioEngine_SDL3)
        target_include_directories(${app_target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
        target_precompile_headers(${app_target} PRIVATE <base/ZEngine.h>)
    endif()
endforeach()

if(ZENGINE_PLATFORM_DARWIN)
    add_executable(ZENGINE_rmlui_gpu_tests EXCLUDE_FROM_ALL tests/RmlUiGpuTests.cpp)
    target_link_libraries(ZENGINE_rmlui_gpu_tests PRIVATE ZENGINE_core ZENGINE_Render_SDL3)
    target_compile_definitions(ZENGINE_rmlui_gpu_tests PRIVATE RMLUI_GPU_FIXTURE="${CMAKE_CURRENT_SOURCE_DIR}/tests/assets/RmlUiProbe.rml")
    target_precompile_headers(ZENGINE_rmlui_gpu_tests PRIVATE <base/ZEngine.h>)
    set_target_properties(ZENGINE_rmlui_gpu_tests PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON FOLDER "test")
endif()
