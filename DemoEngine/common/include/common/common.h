
#pragma once
#include <common/micro.h>
#include <common/Compiler.h>
#include <common/Log.h>
#include <cstdint>
#include <memory>
#include <string>
#include <cassert>

#ifdef _DEBUG
	#define COMMON_ASSERT(val) assert(val)
#else
	#define COMMON_ASSERT(val) 
#endif//_DEBUG

// 某个条件总是成立，那么使用 __assume 可以让编译器基于这个假设进行更激进的优化。
#if defined(DEMOENGINE_COMPILER_MSVC)
	#define COMMON_ASSUME(expr) __assume(expr)
#elif defined(DEMOENGINE_COMPILER_CLANG) || defined(DEMOENGINE_COMPILER_CLANGCL)
	#define COMMON_ASSUME(expr) { const auto b = (expr); __builtin_assume(b); }
#elif defined(DEMOENGINE_COMPILER_GCC)
	#define COMMON_ASSUME(expr) if (!(expr)) { __builtin_unreachable(); }
#endif

namespace CommonWorker
{
    enum VARIANT_TYPE_ENUM
    {
        VTYPE_UNKNOWN,	// 未知
        VTYPE_BOOL,		// 布尔
        VTYPE_INT,		// 32位整数
        VTYPE_INT64,	// 64位整数
        VTYPE_FLOAT,	// 单精度浮点数
        VTYPE_DOUBLE,	// 双精度浮点数
        VTYPE_STRING,	// 字符串
        VTYPE_WIDESTR,	// 宽字符串
        VTYPE_OBJECT,	// 对象号
        VTYPE_POINTER,	// 指针
        VTYPE_USERDATA,	// 用户数据
        VTYPE_TABLE,	// 表
        VTYPE_MAX,
    };


	template <typename To, typename From>
	inline To checked_cast(From* p) 
	{
		COMMON_ASSERT(dynamic_cast<To>(p) == static_cast<To>(p));
		return static_cast<To>(p);
	}
	
	template <typename To, typename From>
	inline To checked_cast(const From* p) 
	{
		COMMON_ASSERT(dynamic_cast<To>(p) == static_cast<To>(p));
		return static_cast<To>(p);
	}

    template <typename T, typename... Args>
	inline std::shared_ptr<T> MakeSharedPtr(Args&&... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

    template <typename T, typename... Args>
	inline std::unique_ptr<T> MakeUniquePtrHelper(std::false_type, Args&&... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

    template <typename T, typename... Args>
	inline std::unique_ptr<T> MakeUniquePtrHelper(std::true_type, size_t size)
	{
		static_assert(0 == std::extent<T>::value,
			"make_unique<T[N]>() is forbidden, please use make_unique<T[]>().");

		return std::make_unique<T>(size);
	}

    template <typename T, typename... Args>
	inline std::unique_ptr<T> MakeUniquePtr(Args&&... args)
	{
		return MakeUniquePtrHelper<T>(std::is_array<T>(), std::forward<Args>(args)...);
	}

    // Unicode函数, 用于string, wstring之间的转换
	std::string& Convert(std::string& dest, std::string_view src);
	std::string& Convert(std::string& dest, std::wstring_view src);
	std::wstring& Convert(std::wstring& dest, std::string_view src);
	std::wstring& Convert(std::wstring& dest, std::wstring_view src);
}

#include <common/defer.h>
#include <common/Log.h>
#include <common/ErrorHandling.h>
