set(GAME_NPC_JSON "${CMAKE_SOURCE_DIR}/ZEngine/Assets/Config/npc.json")
set(GAME_NPC_GEN_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/include/game/generated/NpcConfig.gen.h")
set(GAME_NPC_GEN_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/Game/generated/NpcConfig.gen.cpp")
set(GAME_NPC_GEN_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/Tool/gen_npc_config.py")

find_package(Python3 COMPONENTS Interpreter)

if(Python3_Interpreter_FOUND)
    add_custom_command(
        OUTPUT
            ${GAME_NPC_GEN_HEADER}
            ${GAME_NPC_GEN_SOURCE}
        COMMAND
            ${Python3_EXECUTABLE}
            ${GAME_NPC_GEN_SCRIPT}
            --input ${GAME_NPC_JSON}
            --header ${GAME_NPC_GEN_HEADER}
            --source ${GAME_NPC_GEN_SOURCE}
        DEPENDS
            ${GAME_NPC_JSON}
            ${GAME_NPC_GEN_SCRIPT}
        COMMENT "Generating NpcConfig from npc.json"
        VERBATIM
    )

    add_custom_target(ZENGINE_gen_npc_config
        DEPENDS
            ${GAME_NPC_GEN_HEADER}
            ${GAME_NPC_GEN_SOURCE}
    )
    set_target_properties(ZENGINE_gen_npc_config PROPERTIES FOLDER "Tool")
endif()

set(GAME_SRC_LIST
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Game/DataManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Game/GameContext.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Game/Model.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Game/Scene.cpp
    ${GAME_NPC_GEN_SOURCE}
)

add_library(ZENGINE_game SHARED ${GAME_SRC_LIST})

if(TARGET ZENGINE_gen_npc_config)
    add_dependencies(ZENGINE_game ZENGINE_gen_npc_config)
endif()

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
