if(WIN32)
    if(MSVC AND (CMAKE_GENERATOR MATCHES "^Visual Studio"))
        if((CMAKE_GENERATOR_PLATFORM STREQUAL "x64") OR (CMAKE_GENERATOR MATCHES "Win64"))
            set(DEMOENGINE_ARCH_NAME "x64")
            set(DEMOENGINE_VS_PLATFORM_NAME "x64")
        elseif((CMAKE_GENERATOR_PLATFORM STREQUAL "ARM64") OR (CMAKE_GENERATOR MATCHES "ARM64"))
            set(DEMOENGINE_ARCH_NAME "arm64")
            set(DEMOENGINE_VS_PLATFORM_NAME "ARM64")
        else()
            message(FATAL_ERROR "This CPU architecture is not supported")
        endif()
    endif()
    set(DEMOENGINE_PLATFORM_WINDOWS TRUE)
elseif(ANDROID)
    set(DEMOENGINE_PLATFORM_NAME "android")
    set(DEMOENGINE_PLATFORM_ANDROID TRUE)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(DEMOENGINE_PLATFORM_NAME "Linux")
    set(DEMOENGINE_PLATFORM_LINUX TRUE)
elseif(IOS)
    set(DEMOENGINE_PLATFORM_NAME "ios")
    set(DEMOENGINE_PLATFORM_IOS TRUE)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    if(IOS)
        set(DEMOENGINE_PLATFORM_NAME "ios")
        set(DEMOENGINE_PLATFORM_IOS TRUE)
    else()
        set(DEMOENGINE_PLATFORM_NAME "darwin")
        set(DEMOENGINE_PLATFORM_DARWIN TRUE)
    endif()
endif()

if(DEMOENGINE_PLATFORM_ANDROID OR DEMOENGINE_PLATFORM_WINDOWS)
	set(DEMOENGINE_PREFERRED_LIB_TYPE "STATIC")
else()
	set(DEMOENGINE_PREFERRED_LIB_TYPE "SHARED")
endif()