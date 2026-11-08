# C# is the sole gameplay implementation. There is no C++ GAS fallback.
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/.." "${CMAKE_BINARY_DIR}/gameplay")
target_link_libraries(ZENGINE_game PRIVATE gameplay_dotnet_host)
add_dependencies(ZENGINE_game gameplay_managed)
if(WIN32)
    add_custom_command(TARGET ZENGINE_game POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "$<TARGET_FILE_DIR:gameplay_client>/nethost.dll" "$<TARGET_FILE_DIR:ZENGINE_game>"
        VERBATIM)
    add_dependencies(ZENGINE_game gameplay_client)
endif()
