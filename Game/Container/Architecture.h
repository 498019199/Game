#ifndef STX_ARCHITECTURE_HPP
#define STX_ARCHITECTURE_HPP
#pragma once

#include "macro.h"
// Detects supported CPU architectures
#if defined(STX_COMPILER_MSVC)
	#if defined(_M_IX86)
		#define STX_CPU_X86
		#define STX_COMPILER_TARGET x86
	#elif defined(_M_X64)
		#define STX_CPU_X64
		#define STX_COMPILER_TARGET x64
	#elif defined(_M_ARM64)
		#define STX_CPU_ARM64
		#define STX_COMPILER_TARGET arm64
	#elif defined(_M_ARM)
		#define STX_CPU_ARM
		#define STX_COMPILER_TARGET arm
	#else
		#error "Unknown CPU type. In msvc, x64, arm, and arm64 are supported."
	#endif
#elif defined(STX_COMPILER_GCC) || defined(STX_COMPILER_CLANG)
	#if defined(__x86_64__)
		#define STX_CPU_X64
		#define STX_COMPILER_TARGET x64
	#elif defined(__i386__)
		#define STX_CPU_X86
		#define STX_COMPILER_TARGET x86
	#elif defined(__arm__)
		#define STX_CPU_ARM
		#define STX_COMPILER_TARGET arm
	#elif defined(__aarch64__)
		#define STX_CPU_ARM64
		#define STX_COMPILER_TARGET arm64
	#else
		#error "Unknown CPU type. In g++/clang, x86, x64, arm, and arm64 are supported."
	#endif
#endif

// Detects optional instruction sets
#ifdef STX_CPU_X64
	#define STX_SSE_SUPPORT
	#define STX_SSE2_SUPPORT
	#define STX_X64_SUPPORT
	#if defined(STX_COMPILER_MSVC)
		#ifdef __AVX__
			#define STX_AVX_SUPPORT
		#endif
		#ifdef __AVX2__
			#define STX_AVX2_SUPPORT
		#endif	
	#elif defined(STX_COMPILER_GCC) || defined(STX_COMPILER_CLANG)
		#ifdef __SSE3__
			#define STX_SSE3_SUPPORT
		#endif
		#ifdef __SSSE3__
			#define STX_SSSE3_SUPPORT
		#endif
		#ifdef __SSE4_1__
			#define STX_SSE4_1_SUPPORT
		#endif
		#ifdef __SSE4_2__
			#define STX_SSE4_2_SUPPORT
		#endif
		#ifdef __AVX__
			#define STX_AVX_SUPPORT
		#endif
		#ifdef __AVX2__
			#define STX_AVX2_SUPPORT
		#endif
	#endif
#elif defined KLAYGE_CPU_X86
	#if defined(STX_COMPILER_GCC) || defined(STX_COMPILER_CLANG)
		#ifdef __MMX__
			#define STX_MMX_SUPPORT
		#endif
		#ifdef __SSE__
			#define STX_SSE_SUPPORT
		#endif
		#ifdef __SSE2__
			#define STX_SSE2_SUPPORT
		#endif
		#ifdef __SSE3__
			#define STX_SSE3_SUPPORT
		#endif
		#ifdef __SSSE3__
			#define STX_SSSE3_SUPPORT
		#endif
		#ifdef __SSE4_1__
			#define STX_SSE4_1_SUPPORT
		#endif
		#ifdef __SSE4_2__
			#define STX_SSE4_2_SUPPORT
		#endif
		#ifdef __AVX__
			#define STX_AVX_SUPPORT
		#endif
		#ifdef __AVX2__
			#define STX_AVX2_SUPPORT
		#endif
	#endif
#elif defined STX_CPU_ARM
	#if defined(KLAYGE_COMPILER_MSVC)
		#define STX_NEON_SUPPORT
	#elif defined(STX_COMPILER_GCC) || defined(STX_COMPILER_CLANG)
		#ifdef __ARM_NEON__
			#define STX_NEON_SUPPORT
		#endif
	#endif
#elif defined STX_CPU_ARM64
	#define STX_NEON_SUPPORT
#endif

#endif		// STX_ARCHITECTURE_HPP
