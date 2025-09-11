
#pragma once

#include <common/common.h>

#if defined(DEBUG) | defined(_DEBUG)
	#define ZENGINE_DEBUG
#endif

#define ZENGINE_NAME			ZENGINE
#define ZENGINE_STRINGIZE(X) ZENGINE_DO_STRINGIZE(X)
#define ZENGINE_DO_STRINGIZE(X) #X

#ifdef ZENGINE_EXPORTS		// Build dll
	#define ZENGINE_CORE_API 
//ZENGINE_SYMBOL_EXPORT
#else						// Use dll
	#define ZENGINE_CORE_API 
//ZENGINE_SYMBOL_IMPORT
#endif


#define ZENGINE_NONCOPYABLE(T) \
T(T const& rhs) = delete; \
T& operator=(T const& rhs) = delete;

#define ZENGINE_NONMOVEABLE(T) \
T(T&& rhs) = delete; \
T& operator=(T&& rhs) = delete;

#include <base/Context.h>