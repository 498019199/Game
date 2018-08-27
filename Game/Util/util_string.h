#ifndef _UTIL_STRING_H
#define _UTIL_STRING_H
#include "../Container/var_type.h"
#include "../Container/persistid.h"
#include "../Platform/system_table.h"
#include "../Tool/UtilString.h"
#include<tchar.h>
#include <time.h>

#pragma once

// 去前后空白符
inline void trim_string1(char* szNew, const char* szOld, size_t nMaxSize)
{
	char* p = const_cast<char*>(szOld);
	char* q = const_cast<char*>(szOld);
	while (' ' == *p || '\t' == *p) ++p;
	while (*q++ = *p++);
	q -= 2;
	while (' ' == *q || '\t' == *q) --q;


	size_t nCount = q - szOld;
	if (nCount < nMaxSize)
	{
		memcpy(szNew, szOld, nCount);
	}
	else
	{
		memcpy(szNew, szOld, nMaxSize);
	}
}

// 去前后空白符
inline void trim_string(char* szNew, const char* szOld)
{
	return trim_string1(szNew, szOld, UtilString::length(szOld));
}

inline int64_t GetUniqueID()
{
	GUID id;
	if (S_OK  != CoCreateGuid(&id))
	{
		return 0;
	}

	uint64_t res = (uint64_t)id.Data1 << 40 | (uint64_t)id.Data2 << 24 | (uint64_t)id.Data3 << 8 | (uint64_t)id.Data4[0] << 7 |
		(uint64_t)id.Data4[1] << 6 | (uint64_t)id.Data4[2] << 5 | (uint64_t)id.Data4[3] << 4 | (uint64_t)id.Data4[4] << 3 |
		(uint64_t)id.Data4[5] << 2 | (uint64_t)id.Data4[6] << 1 | (uint64_t)id.Data4[7];

	return static_cast<int64_t>(res);
}
//#define MAKE_INT64(a, b) static_cast<>(a)
//CoCreateGuid

#endif//_UTIL_STRING_H