// 2018年8月29日 异常函数集 zhangbei
#ifndef _UTIL_ERROR_HANDLE_
#define _UTIL_ERROR_HANDLE_
#pragma once
#include "../Container/C++17/string_view.h"
#include <string>
#include <stdexcept>
std::string CombineFileLine(std::string_view file, int line);
void Verify(bool x);

// Throw error code
#define TEC(x)			{ throw std::system_error(x, CombineFileLine(__FILE__, __LINE__)); }

// Throw error message
#define TMSG(msg)		{ throw std::runtime_error(msg); }

// Throw if failed (error code)
#define TIFEC(x)		{ if (x) TEC(x) }

// Throw if failed (errc)
#define TERRC(x)		TEC(std::make_error_code(x))

// Throw if failed (errc)
#define TIFERRC(x)		TIFEC(std::make_error_code(x))

// Throw if failed (HRESULT)
#define TIFHR(x)		{ if (static_cast<HRESULT>(x) < 0) { TMSG(CombineFileLine(__FILE__, __LINE__)); } }
#endif//_UTIL_ERROR_HANDLE_


