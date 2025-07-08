#pragma once

#if defined(__clang__)
    #define CLANG_VERSION KFL_JOIN(__clang_major__, __clang_minor__)

	#if __cplusplus < 202002L
		#error "-std=c++20 must be turned on."
	#endif

	#if defined(_MSC_VER)
		#define ZENGINE_COMPILER_CLANGCL
		#define ZENGINE_COMPILER_NAME clangcl

		#if CLANG_VERSION >= 120
			#define ZENGINE_COMPILER_VERSION CLANG_VERSION
		#else
			#error "Unsupported compiler version. Please install clang-cl 12.0 or up."
		#endif
	#else
		#define ZENGINE_COMPILER_CLANG
		#define ZENGINE_COMPILER_NAME clang

		#if defined(__APPLE__)
			#if CLANG_VERSION >= 130
				#define ZENGINE_COMPILER_VERSION CLANG_VERSION
			#else
				#error "Unsupported compiler version. Please install Apple clang++ 13 or up."
			#endif
		#elif defined(__ANDROID__)
			#if CLANG_VERSION >= 120
				#define ZENGINE_COMPILER_VERSION CLANG_VERSION
			#else
				#error "Unsupported compiler version. Please install clang++ 12.0 (NDK 23c) or up."
			#endif
		#elif defined(linux) || defined(__linux) || defined(__linux__)
			#if CLANG_VERSION >= 140
				#define ZENGINE_COMPILER_VERSION CLANG_VERSION
			#else
				#error "Unsupported compiler version. Please install clang++ 14.0 or up."
			#endif
		#else
			#error "Clang++ on an unknown platform. Only Apple, Android, and Linux are supported."
		#endif
	#endif
#elif defined(_MSC_VER)
	#define ZENGINE_COMPILER_MSVC
	#define ZENGINE_COMPILER_NAME vc

	#if _MSC_VER >= 1930
		#define ZENGINE_COMPILER_VERSION 143
	#elif _MSC_VER >= 1920
		#define ZENGINE_COMPILER_VERSION 142
	#else
		#error "Unsupported compiler version. Please install VS2019 or up."
	#endif

	#if __cplusplus < 201703L
		#error "/std:c++17 must be turned on."
	#endif

	#pragma warning(disable : 4251) // STL classes are not dllexport.
	#pragma warning(disable : 4819) // Allow non-ANSI characters.
#elif defined(__GNUC__)
    #define ZENGINE_COMPILER_GCC
	#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
		#define ZENGINE_COMPILER_NAME mgw
	#else
		#define ZENGINE_COMPILER_NAME gcc
	#endif

    #define GCC_VERSION KFL_JOIN(__GNUC__, __GNUC_MINOR__)
	#if GCC_VERSION >= 110
		#define ZENGINE_COMPILER_VERSION GCC_VERSION
	#else
		#error "Unsupported compiler version. Please install g++ 11.0 or up."
	#endif

	#if __cplusplus < 202002L
		#error "-std=c++20 must be turned on."
	#endif
#else
	#error "Unknown compiler. Please install vc, g++, or clang."
#endif