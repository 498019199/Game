include(CTest)
if(BUILD_TESTING)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    set(TEST_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/tests")
    add_custom_target(gameplay_test_build ALL
        COMMAND "${DOTNET_EXECUTABLE}" publish tests/Game.Tests.csproj -c Release -o "${TEST_OUTPUT}" --nologo
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" DEPENDS gameplay_managed VERBATIM)
    add_test(NAME gameplay_protocol_parity COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/verify.py"
        --client "$<TARGET_FILE:gameplay_client>" --managed "${MANAGED_OUTPUT}" --dotnet "${DOTNET_EXECUTABLE}"
        --server "${SERVER_OUTPUT}/Game.Server.dll" --tests "${TEST_OUTPUT}/Game.Tests.dll"
        --work "${CMAKE_CURRENT_BINARY_DIR}/verification")
    set_tests_properties(gameplay_protocol_parity PROPERTIES
        ENVIRONMENT "DOTNET_ROOT=${DOTNET_ROOT};DOTNET_CLI_TELEMETRY_OPTOUT=1;GAMEPLAY_MANAGED_DIR=${MANAGED_OUTPUT}"
        TIMEOUT 120)
endif()
