set(GAME_SRC_LIST
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Model.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Scene.cpp
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
