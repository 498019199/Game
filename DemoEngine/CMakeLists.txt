cmake_minimum_required(VERSION 3.5)
project(DemoEngine C CXX)

set(CMAKE_DEBUG_POSTFIX "_d" CACHE STRING "")
set(3RD_DEBUG_POSTFIX "d" CACHE STRING "")
set(CMAKE_RELEASE_POSTFIX "" CACHE STRING "")

include(${PROJECT_SOURCE_DIR}/cmake/Compiler.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/Platform.cmake)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(PROJECT_3RD_DIR ${PROJECT_SOURCE_DIR}/3rd)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    # windows debug or release
    if(NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(LIB_DIR_PATH ${LIBRARY_OUTPUT_PATH}/Debug)
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(LIB_DIR_PATH ${LIBRARY_OUTPUT_PATH}/Release)
    endif()
endif()

SET(DEMOENGINE_IS_DEV_PLATFORM TRUE)

add_subdirectory(3rd)
add_subdirectory(common)
file(COPY ${PROJECT_SOURCE_DIR}/common/include DESTINATION "${LIBRARY_OUTPUT_PATH}")
add_subdirectory(math)
file(COPY ${PROJECT_SOURCE_DIR}/math/include DESTINATION "${LIBRARY_OUTPUT_PATH}")
add_subdirectory(core)
file(COPY ${PROJECT_SOURCE_DIR}/core/include DESTINATION "${LIBRARY_OUTPUT_PATH}")

