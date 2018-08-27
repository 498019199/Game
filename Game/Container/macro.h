#ifndef _STX_MACRO_H_
#define _STX_MACRO_H_
#pragma once

# if defined(_MSC_VER)
    # ifndef _CRT_SECURE_NO_DEPRECATE
        # define _CRT_SECURE_NO_DEPRECATE (1)
    # endif
    #pragma warning (disable:4521) 
    #pragma warning(disable : 4996)
# endif

#ifdef _WIN32
    #define STX_PLATFORM_WIN
    #if _WIN64
        #define STX_PLATFORM_WIN64
    #else
        #define STX_PLATFORM_WIN32
    #endif
#endif

#ifdef _GUN_
    #define STX_PLATFORM_IOS
    #define STX_PLATFORM_IOS32
    #define STX_PLATFORM_IOS64
#endif

#ifdef _linux_
    #define STX_PLATFORM_LINUX
    #define STX_PLATFORM_LINUX32
    #define STX_PLATFORM_LINUX64
#endif

#ifdef _ANDDROD_
#define STX_PLATFORM_ANDROID
#define STX_PLATFORM_ANDROID32
#define STX_PLATFORM_ANDROID64
#endif

// 支持C++17 宏
// STX_TS_LIBRARY_FILESYSTEM_SUPPORT  支持#include <experimental/filesystem>
// STX_CXX17_LIBRARY_FILESYSTEM_SUPPORT  支持#include <filesystem>
// STX_CXX17_LIBRARY_STRING_VIEW_SUPPORT 支持#include <string_view>

#if _MSVC_LANG > 201402
	#if _MSC_VER >= 1911
		#define STX_CXX17_CORE_IF_CONSTEXPR_SUPPORT
	#endif
	#if _MSC_VER >= 1910
		#define STX_CXX17_CORE_STATIC_ASSERT_V2_SUPPORT
	#endif
		#define STX_CXX17_LIBRARY_ANY_SUPPORT
	#if _MSC_VER >= 1914
		#define STX_CXX17_LIBRARY_FILESYSTEM_SUPPORT
	#endif
	#define STX_CXX17_LIBRARY_OPTIONAL_SUPPORT
	#define STX_CXX17_LIBRARY_STRING_VIEW_SUPPORT
#endif

#define STX_CXX17_LIBRARY_SIZE_AND_MORE_SUPPORT
#if !defined(STX_CXX17_LIBRARY_FILESYSTEM_SUPPORT)
#define STX_TS_LIBRARY_FILESYSTEM_SUPPORT
#endif
#define STX_COMPILER_CLANGC2
#if  defined(_DEBUG) 
    #define  DEBUG_STATE 1
#endif//_DEBUG
#endif//_STX_MACRO_H_