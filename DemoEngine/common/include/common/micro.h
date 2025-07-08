#pragma once

#if defined(__clang__)
    #if __cplusplus < 202002L
		#error "-std=c++20 must be turned on."
	#endif
#elif defined(__GNUC__)
    #if __cplusplus < 202002L
		#error "-std=c++20 must be turned on."
	#endif
#elif defined(_MSC_VER)
    #if __cplusplus < 201703L
	    #error "/std:c++17 must be turned on."
	#endif

    #if _MSC_VER >= 1930
		#define ZENGINE_COMPILER_VERSION 143
	#elif _MSC_VER >= 1920
		#define ZENGINE_COMPILER_VERSION 142
	#else
		#error "Unsupported compiler version. Please install VS2019 or up."
	#endif

	#pragma warning(disable : 4251) // STL classes are not dllexport.
	#pragma warning(disable : 4819) // Allow non-ANSI characters.
#else
    #error "Unknown compiler. Please install vc, g++, or clang."
#endif


#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
	// Shut min/max in windows.h
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif
	
    #define ZENGINE_PLATFORM_WINDOWS
	#if defined(_WIN64)
		#define ZENGINE_PLATFORM_WIN64
	#else
		#define ZENGINE_PLATFORM_WIN32
	#endif
#elif defined(__ANDROID__)
    #define ZENGINE_PLATFORM_ANDROID
#elif defined(linux) || defined(__linux) || defined(__linux__)
    #define ZENGINE_PLATFORM_LINUX
#elif defined(__APPLE__)
    #include <TargetConditionals.h>
	#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
		#define ZENGINE_PLATFORM_IOS
	#else
		#define ZENGINE_PLATFORM_DARWIN
	#endif
#else
	#error "Unknown platform. The supported target platforms are Windows, Android, Linux, macOS, and iOS."
#endif


#if (_WIN32_WINNT >= _WIN32_WINNT_WIN10)
	#include <winapifamily.h>
	#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY != WINAPI_FAMILY_DESKTOP_APP)
		#ifndef NTDDI_WIN10_RS4
			#error "You need to install Windows SDK 10.0.17133.0 or up to build UWP."
		#endif
		#define ZENGINE_PLATFORM_WINDOWS_STORE
	#else
		#define ZENGINE_PLATFORM_WINDOWS_DESKTOP
	#endif
#else
	#define ZENGINE_PLATFORM_WINDOWS_DESKTOP
#endif


