# Both languages use the exact same pinned protoc/runtime release. Check the
# committed C# output used by Docker against freshly generated code.
add_custom_target(gameplay_managed ALL
    COMMAND "${CMAKE_COMMAND}" -E compare_files "${GENERATED}/csharp/Gameplay.cs" "${CMAKE_CURRENT_SOURCE_DIR}/protocol/generated/Gameplay.cs"
    COMMAND "${DOTNET_EXECUTABLE}" publish bridge/Game.NativeBridge.csproj -c Release -o "${MANAGED_OUTPUT}" --nologo
    COMMAND "${DOTNET_EXECUTABLE}" publish server/Game.Server.csproj -c Release -o "${SERVER_OUTPUT}" --nologo
    COMMAND "${DOTNET_EXECUTABLE}" "${SERVER_OUTPUT}/Game.Server.dll" --bake
        "${CMAKE_CURRENT_SOURCE_DIR}/../zengine/assets/config" "${SERVER_OUTPUT}/game.catalog.pb"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${SERVER_OUTPUT}/game.catalog.pb" "${MANAGED_OUTPUT}/game.catalog.pb"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    DEPENDS gameplay_codegen
    COMMENT "Building C# gameplay and compiling protobuf configuration"
    VERBATIM)
