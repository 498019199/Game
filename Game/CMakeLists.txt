set(GAME_SRC_LIST
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Manager/DataManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Data/generated/LoadPrefabData.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Game/GameContext.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Game/Model.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Game/Scene.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Game/NpcSpawner.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Game/gas/GameplayTag.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Game/gas/AttributeSet.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Game/gas/EffectContainer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Game/gas/AbilitySystemComponent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Game/gas/CombatBeat.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Game/gas/CombatService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Game/gas/SkillCatalog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Game/gas/SkillPipeline.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UI/GmDebugWindow.cpp
)

add_library(ZENGINE_game SHARED ${GAME_SRC_LIST})

target_include_directories(ZENGINE_game
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${ZENGINE_ROOT_HEADER}
        ${DEMOENGINE_DIR}/common/include
        ${DEMOENGINE_DIR}/core/core/include
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
)

if(ZENGINE_IS_DEV_PLATFORM)
    add_subdirectory(Tool)
endif()
