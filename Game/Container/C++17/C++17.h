#ifndef _STX_CXX17_H_
#define _STX_CXX17_H_

#pragma once

#ifdef STX_CXX17_CORE_IF_CONSTEXPR_SUPPORT
	#define STX_IF_CONSTEXPR(x) if constexpr (x)
#else
	#ifdef STX_COMPILER_MSVC
		#pragma warning(disable: 4127)
	#endif
	#define STX_IF_CONSTEXPR(x) if (x)
#endif

#ifdef STX_CXX17_CORE_STATIC_ASSERT_V2_SUPPORT
	#define STX_STATIC_ASSERT(x) static_assert(x)
#else
	#define STX_STATIC_ASSERT(x) static_assert(x, #x)
#endif

#endif		// _STX_CXX17_H_
