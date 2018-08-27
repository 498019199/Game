// 2018年8月1日
#ifndef _KFL_CXX17_OPTIONAL_HPP
#define _KFL_CXX17_OPTIONAL_HPP
#pragma once
#include "../Container/macro.h"

#if defined(STX_CXX17_LIBRARY_OPTIONAL_SUPPORT)
	#include <optional>
#elif defined(STX_TS_LIBRARY_OPTIONAL_SUPPORT)
	#include <experimental/optional>
	namespace std
	{
		using experimental::optional;
	}
#else
//#ifdef STX_COMPILER_CLANGC2
//#pragma clang diagnostic push
//#pragma clang diagnostic ignored "-Wunused-parameter" // Ignore unused parameter 'out', 'v'
//#endif
// 不知道为什么boost,报错误，只能切换到C++17聊了。2018年8月1日
// boost有些头文件没有替换，导致boost文件有些版本不一致的😭
#include <boost/optional.hpp>
//#ifdef STX_COMPILER_CLANGC2
//#pragma clang diagnostic pop
//#endif
namespace std
{
	using boost::optional;
}
#endif

#endif		// _KFL_CXX17_OPTIONAL_HPP
