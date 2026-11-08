set(GENERATED "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${GENERATED}/cpp" "${GENERATED}/csharp")
add_custom_command(
    OUTPUT "${GENERATED}/cpp/gameplay.pb.cc" "${GENERATED}/cpp/gameplay.pb.h" "${GENERATED}/csharp/Gameplay.cs"
    COMMAND $<TARGET_FILE:protobuf::protoc> -I "${CMAKE_CURRENT_SOURCE_DIR}/proto"
        --cpp_out=${GENERATED}/cpp --csharp_out=${GENERATED}/csharp "${CMAKE_CURRENT_SOURCE_DIR}/proto/gameplay.proto"
    DEPENDS protobuf::protoc "${CMAKE_CURRENT_SOURCE_DIR}/proto/gameplay.proto"
    VERBATIM)
add_custom_target(gameplay_codegen DEPENDS "${GENERATED}/cpp/gameplay.pb.cc" "${GENERATED}/csharp/Gameplay.cs")
add_custom_target(gameplay_codegen_refresh
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${GENERATED}/csharp/Gameplay.cs" "${CMAKE_CURRENT_SOURCE_DIR}/protocol/generated/Gameplay.cs"
    DEPENDS gameplay_codegen VERBATIM)
add_library(gameplay_protocol STATIC "${GENERATED}/cpp/gameplay.pb.cc")
set_target_properties(gameplay_protocol PROPERTIES POSITION_INDEPENDENT_CODE ON)
# Generated .pb.cc is third-party output; handwritten host/client code keeps
# the normal project warning level.
if(MSVC)
    target_compile_options(gameplay_protocol PRIVATE /w)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(gameplay_protocol PRIVATE -w)
endif()
target_include_directories(gameplay_protocol SYSTEM PUBLIC "${GENERATED}/cpp")
target_link_libraries(gameplay_protocol PUBLIC protobuf::libprotobuf)
target_compile_features(gameplay_protocol PUBLIC cxx_std_17)
