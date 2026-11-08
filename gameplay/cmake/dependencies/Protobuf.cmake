include(FetchContent)

# PRIVATE flags silence dependency/generated sources only. SYSTEM interfaces
# also keep their headers quiet when included by engine/game code.
function(gameplay_quiet_dependency directory)
    get_property(targets DIRECTORY "${directory}" PROPERTY BUILDSYSTEM_TARGETS)
    foreach(dependency IN LISTS targets)
        get_target_property(kind "${dependency}" TYPE)
        if(kind STREQUAL "UTILITY")
            continue()
        endif()
        get_target_property(headers "${dependency}" INTERFACE_INCLUDE_DIRECTORIES)
        if(headers)
            set_property(TARGET "${dependency}" APPEND PROPERTY
                INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${headers}")
        endif()
        if(NOT kind STREQUAL "INTERFACE_LIBRARY")
            if(MSVC)
                target_compile_options("${dependency}" PRIVATE /w)
            elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
                target_compile_options("${dependency}" PRIVATE -w)
            endif()
        endif()
    endforeach()
    get_property(children DIRECTORY "${directory}" PROPERTY SUBDIRECTORIES)
    foreach(child IN LISTS children)
        gameplay_quiet_dependency("${child}")
    endforeach()
endfunction()

function(gameplay_protobuf_dependencies)
    # Keep third-party warning/options local; never alter the engine toolchain.
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    set(CMAKE_CXX_STANDARD 17)
    # Protobuf explicitly selects an old policy; do not suppress deprecation
    # diagnostics outside this dependency configuration scope.
    cmake_policy(PUSH)
    if(POLICY CMP0218)
        # FetchContent functions capture diagnostic state at definition time;
        # use the scoped compatibility variable for these older dependencies.
        cmake_policy(SET CMP0218 OLD)
    endif()
    set(CMAKE_WARN_DEPRECATED OFF)
    foreach(policy CMP0156 CMP0179)
        if(POLICY ${policy})
            set(CMAKE_POLICY_DEFAULT_${policy} NEW)
        endif()
    endforeach()
    set(ABSL_PROPAGATE_CXX_STD ON)
    set(ABSL_ENABLE_INSTALL OFF)
    FetchContent_Declare(absl
        URL https://codeload.github.com/abseil/abseil-cpp/tar.gz/refs/tags/20250512.1
        URL_HASH SHA256=9b7a064305e9fd94d124ffa6cc358592eb42b5da588fb4e07d09254aa40086db
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    FetchContent_MakeAvailable(absl)
    set(protobuf_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(protobuf_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(protobuf_INSTALL OFF CACHE BOOL "" FORCE)
    set(protobuf_WITH_ZLIB OFF CACHE BOOL "" FORCE)
    set(protobuf_LOCAL_DEPENDENCIES_ONLY ON CACHE BOOL "" FORCE)
    FetchContent_Declare(protobuf
        URL https://codeload.github.com/protocolbuffers/protobuf/tar.gz/refs/tags/v36.2
        URL_HASH SHA256=7e7eaf580ec77f1a392ff1ab1013c13d3b04a6716f002f697f67f3c1113f0530
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    FetchContent_MakeAvailable(protobuf)
    gameplay_quiet_dependency("${absl_SOURCE_DIR}")
    gameplay_quiet_dependency("${protobuf_SOURCE_DIR}")
    cmake_policy(POP)
endfunction()
gameplay_protobuf_dependencies()
