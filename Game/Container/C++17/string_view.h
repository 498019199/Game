#ifndef _STX_STRING_VIEW_H_
#define _STX_STRING_VIEW_H_
#pragma once
#include "../Container/macro.h"

#if defined(STX_CXX17_LIBRARY_STRING_VIEW_SUPPORT)
#include <string_view>
#else
#include <boost/utility/string_view.hpp>
namespace std
{
	using boost::basic_string_view;
	using boost::string_view;
	using boost::wstring_view;
}
#endif
#endif//_STX_STRING_VIEW_H_
