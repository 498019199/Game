cmake_minimum_required(VERSION 3.24)
project(GameplayManaged LANGUAGES C CXX)

# Let capable linkers de-duplicate Protobuf/Abseil static libraries instead of
# masking linker warnings globally. Guard policies for CMake 3.24 compatibility.
foreach(policy CMP0156 CMP0179)
    if(POLICY ${policy})
        cmake_policy(SET ${policy} NEW)
    endif()
endforeach()

# Standalone build: intentionally does not enter engine/ or editor/.

# Configure the runtime, then define targets in dependency order.
include("${CMAKE_CURRENT_LIST_DIR}/cmake/config/Dotnet.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/dependencies/Protobuf.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/targets/Protocol.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/targets/Managed.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/targets/Native.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/targets/Tests.cmake")
