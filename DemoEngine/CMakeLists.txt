cmake_minimum_required(VERSION 3.5)
project(DemoEngine C CXX)

include(${PROJECT_SOURCE_DIR}/cmake/Compiler.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/Platform.cmake)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_CXX_STANDARD 20)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(PROJECT_3RD_DIR ${PROJECT_SOURCE_DIR}/3rd)
set(ZENGINE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    if(WIN7_SYSTEM_SUPPORT MATCHES ON)
        add_compile_definitions(_WIN32_WINNT=0x601)
    endif()
    
    # windows debug or release
    if(NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(LIB_DIR_PATH ${LIBRARY_OUTPUT_PATH}/Debug)
        set(3RD_DEBUG_POSTFIX "d" CACHE STRING "")
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(LIB_DIR_PATH ${LIBRARY_OUTPUT_PATH}/Release)
    endif()
endif()

add_subdirectory(3rd)
add_subdirectory(common)
file(COPY ${PROJECT_SOURCE_DIR}/common/include DESTINATION "${LIBRARY_OUTPUT_PATH}")
add_subdirectory(math)
file(COPY ${PROJECT_SOURCE_DIR}/math/include DESTINATION "${LIBRARY_OUTPUT_PATH}")
add_subdirectory(core)
file(COPY ${PROJECT_SOURCE_DIR}/core/include DESTINATION "${LIBRARY_OUTPUT_PATH}")

