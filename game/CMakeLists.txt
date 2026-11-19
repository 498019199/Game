set(GAME_SRC_LIST
    ${CMAKE_CURRENT_SOURCE_DIR}/src/manager/DataManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/data/generated/LoadPrefabData.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/game/GameContext.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/game/Model.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/game/Scene.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/game/NpcSpawner.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/GmDebugWindow.cpp
)

add_library(ZENGINE_game SHARED ${GAME_SRC_LIST})

target_include_directories(ZENGINE_game
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_definitions(ZENGINE_game
    PRIVATE
        GAME_EXPORTS
)

set_target_properties(ZENGINE_game PROPERTIES
    OUTPUT_NAME ZENGINE_game${ZENGINE_OUTPUT_SUFFIX}
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

if(MSVC)
    target_compile_options(ZENGINE_game PRIVATE /std:c++20)
endif()

target_link_libraries(ZENGINE_game
    PUBLIC
        ZENGINE_core
        ZENGINE_common
    PRIVATE
        ZENGINE_RmlUiHeaders
)

# Choose a PCH locally; linking core does not impose it on consumers.
target_precompile_headers(ZENGINE_game PRIVATE <base/ZEngine.h>)

if(ZENGINE_ENABLE_MANAGED_GAMEPLAY)
    target_compile_definitions(ZENGINE_game PUBLIC ZENGINE_ENABLE_MANAGED_GAMEPLAY)
    include("${CMAKE_CURRENT_SOURCE_DIR}/../gameplay/cmake/EngineIntegration.cmake")
endif()

if(ZENGINE_IS_DEV_PLATFORM)
    add_subdirectory(tool)
endif()
