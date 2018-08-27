#pragma once
#ifndef _TCHAR_TRAIN_H
#define _TCHAR_TRAIN_H
#include <cwchar>
#include <cstring>
template<typename TYPE>
struct TCharTrains
{
};

template<>
struct TCharTrains<char>
{
	static std::size_t Length(const char* szValue)
	{
		return strlen(szValue);
	}


	static bool Compare(const char* s1, const char* s2)
	{
		return strcmp(s1, s2);
	}

	static void Copy(char* szdest, const char* szsur, const int len)
	{
		memcpy((void*)szdest, szsur, len);
	}

	static void Put(char* szValue, char ch)
	{
		*szValue = ch;
	}

	static std::size_t Find(const char* szdest, const char* find, const int begin = 0)
	{
		const char* szpos = strstr(&szdest[begin], find);
		if (NULL == szpos)
		{
			return std::size_t(-1);
		}
		return std::size_t(szdest - szpos);
	}

	static const char* Empty_Value()
	{
		return "";
	}

	static std::size_t uPos()
	{
		return std::size_t(-1);
	}
};

template<>
struct TCharTrains<wchar_t>
{
	std::size_t Length(const wchar_t* szValue)
	{
		return wcslen(szValue);
	}

	static bool Compare(const wchar_t* s1, const wchar_t* s2)
	{
		return wcscmp(s1, s2);
	}

	static void Copy(wchar_t* szdest, const wchar_t* szsur, const int len)
	{
		memcpy((void*)szdest, szsur, len);
	}

	static void Put(wchar_t* szValue, wchar_t ch)
	{
		*szValue = ch;
	}

	static std::size_t Find(const wchar_t* szdest, const wchar_t* find, const int begin = 0)
	{
		const wchar_t* szpos = wcsstr(&szdest[begin], find);
		if (NULL == szpos)
		{
			return std::size_t(-1);
		}
		return std::size_t(szdest - szpos);
	}

	static wchar_t* Empty_Value()
	{
		return L"";
	}

	static std::size_t uPos()
	{
		return std::size_t(-1);
	}
};
#endif//_TCHAR_TRAIN_H