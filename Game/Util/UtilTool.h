// 2018年7月21日 工具函数集 zhangbei
#ifndef _UTIL_TOOL_
#define _UTIL_TOOL_
#pragma once
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#define STX_MIN(a, b) (((a)<(b))? (a): (b))
#define STX_MAX(a, b) (((a)>(b))? (a): (b))
#define PROTEST_ZERO(v) ((0 == v)? 1: v)
#define NULL_RETURN(v, r) if(nullptr == v){ return r; }
#define NULL_RETURN_VOID(v) if(nullptr == v) { return; }
#define IF_BREAK(a) if(a) { break; }
#define IF_CONTINUE(a) if(!(a)) { continue; }
#define CEILING_DIV_2(len) ((((len) + 1)&(~(1)))/2)
#define STRING_EQUIP(s1, s2) (s1 && s2 && 0 == strcmp(s1, s2))
#define STRWIDE_EQUIP(s1, s2) (s1 && s2 && 0 == wcscmp(s1, s2))
#define ADRESS_EQUIP(v1, v2) (reinterpret_cast<void*>(v1) == reinterpret_cast<void*>(v1))
#define ADRESS_NOEQUIP(v1, v2) (reinterpret_cast<void*>(v1) != reinterpret_cast<void*>(v1))
#define UNUSED(x) (void)(x)

inline void AssertionFail(const char* szMsg, const char* szFile, const char* szFunction, unsigned int nLine);
#define UNREACHABLE_MSG(msg) \
AssertionFail(msg, __FILE__, __FUNCTION__, __LINE__); \
abort(); 

inline void AssertionFail(const char* szMsg, const char* szFile, const char* szFunction, unsigned int nLine)
{
	FILE *fp = NULL;
	fopen_s(&fp, "assert.log", "ab");
	if (NULL != fp)
	{
		fprintf(fp, "assert:%s, file:%s, line:%d funcation:%s\t\n", szMsg, szFile, nLine, szFunction);
		fflush(stdout);
		fclose(fp);
	}
	fprintf(stderr, "assert:%s, file:%s, line:%d funcation:%s\t\n", szMsg, szFile, nLine, szFunction);
	fflush(stderr);
}

template <typename T>
inline std::shared_ptr<T>
MakeCOMPtr(T* p)
{
	return p ? std::shared_ptr<T>(p, std::mem_fn(&T::Release)) : std::shared_ptr<T>();
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

// 打印日志
void fm_log(const char* szFormat, ...);
// 打印调试信息
void trace_log(const char* szFormat, ...);

#if (_MSC_VER > 1700) 
#define STX_STRCPY(des, src) \
        strcpy_s(des,sizeof(src) / sizeof(src[0]), src)
#define STX_WCSCPY(des, src) \
        wcscpy_s(des,sizeof(src), src)
#define STX_SPRINTF(buf, fmt, val) \
        sprintf_s(buf, sizeof(buf), fmt, val)
#endif

#if __linux__ 
#define STX_STRCPY(des, src) \
            strncpy(des,sizeof(src), src)
#define STX_WCSCPY(des, src) \
            wcsncpy(des,sizeof(src), src)
#define STX_SPRINTF(buff, fmt, val) \
            snprintf(buff, sizeof(buf), fmt, val)
#define STX_VSNPRINTF(buf, nsize, fmt, val) \   
			vswprintf(buf, nsize, fmt, val)
#endif
#endif//_UTIL_TOOL_


