cmake_minimum_required(VERSION 3.0)
project(server)

#c++17
set(CMAKE_CXX_STANDARD 17)
#c
set(CMAKE_C_STANDARD 11)

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    message(STATUS "current platform: Linux")
    add_definitions(-DPLATFORM_LINUX)
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
    message(STATUS "current platform: Windows")
    add_definitions(-DPLATFORM_WINDOWS)  
elseif(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
    message(STATUS "current platform: Mac-OS")
    add_definitions(-DPLATFORM_MACOS) 
else()
    message(STATUS "other platform: ${CMAKE_SYSTEM_NAME}")
endif(CMAKE_SYSTEM_NAME MATCHES "Linux")


include_directories(
     ${PROJECT_SOURCE_DIR}/NetModule/*.h)

# 共享库
set(HEADERS
    ${PROJECT_SOURCE_DIR}/NetModule/*.h)

add_executable(${PROJECT_NAME}
    main.cpp
)