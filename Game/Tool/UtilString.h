#ifndef _STX_UTIL_STRING_H
#define _STX_UTIL_STRING_H
#pragma once
#include "../public/var_type.h"
#include <string>
#include <vector>
#define STD std::

#define SafePrint(buf, fmt, val) \
    UtilString::safe_sprinf(buf, sizeof(buf), fmt, val)
#define SafeWprint(buf, fmt, val) \
    UtilString::safe_wsprinf(buf, sizeof(buf), fmt, val)
class UtilString
{
public:
    //字符串转换int
    //\@ src    字符串
    //\@ start  开始位置（默认从0开始）
    //\@ base   进制（默认十进制）
    static int as_int(const char* src, std::size_t *start = 0, int base = 10);

    //字符串转换int64
    //\@ src    字符串
    //\@ start  开始位置（默认从0开始）
    //\@ base   进制（默认十进制）
    static int64_t as_int64(const char* src, std::size_t *start = 0, int base = 10);

    //字符串转换float
    //\@ src    字符串
    //\@ start  开始位置（默认从0开始）
    //\@ base   进制（默认十进制）
    static float as_float(const char* src, std::size_t *start = 0);

    //字符串转换double
    //\@ src    字符串
    //\@ start  开始位置（默认从0开始）
    //\@ base   进制（默认十进制）
    static double as_double(const char* src, std::size_t *start = 0);

    //int转换字符串
    //\@ val    值
    static STD string as_string(int val);

    //int64_t转换字符串
    //\@ val    值
    static STD string as_string(int64_t val);
    
    //float转换字符串
    //\@ val    值
    static STD string as_string(float val);
    
    //double转换字符串
    //\@ val    值
    static STD string as_string(double val);
    
    //宽字符转换字符串
    //\@ val    值
    static STD string as_string(const wchar_t* val);

    //各种基础类型拼接
    //\@ buf    存储的缓存区域
    //\@ nsize  存储大小
    //\@ fmt    拼接的格式（%d）
    //\@ ...    
    static std::size_t safe_sprinf(char* buf, std::size_t nsize, const char* fmt, ...);
    
    //是否相等
    static bool compare(const char* s1, const char* s2);

    //是否是空串
    static bool empty(const char* buf);

    //字符串长度
    static std::size_t length(const char* buf);

    //获取哈希值
    static std::size_t hash_value(const char* szKey);

    //字符串前后去空格
    static STD string div_space(STD string buf);

    //字符串转宽字符
    //ansi转utf8
    //utf8转utf16
    //utf16转utf8
#pragma region 
    //各种基础类型拼接
    //\@ buf    存储的缓存区域
    //\@ nsize  存储大小
    //\@ fmt    拼接的格式（%d）
    //\@ ...    
    static void safe_wsprinf(const wchar_t* buf, std::size_t nsize, const char* fmt, ...);

    //int转换宽符串
    //\@ val    值
    static STD wstring as_wstring(const int val);

    //int64_t转换宽符串
    //\@ val    值
    static STD wstring as_wstring(const int64_t val);

    //float转换宽符串
    //\@ val    值
    static STD wstring as_wstring(const float val);

    //double转换宽符串
    //\@ val    值
    static STD wstring as_wstring(const double val);

    //字符串转换宽符串
    //\@ val    值
    static STD wstring as_wstring(const char* val);

    //宽符串转换int
    //\@ src    字符串
    //\@ start  开始位置（默认从0开始）
    //\@ base   进制（默认十进制）
    static int as_int(const wchar_t* src, std::size_t *start = 0, int base = 10);

    //宽符串转换int64
    //\@ src    字符串
    //\@ start  开始位置（默认从0开始）
    //\@ base   进制（默认十进制）
    static int64_t as_int64(const wchar_t* src, std::size_t *start = 0, int base = 10);

    //宽符串转换float
    //\@ src    字符串
    //\@ start  开始位置（默认从0开始）
    //\@ base   进制（默认十进制）
    static float as_float(const wchar_t* src, std::size_t *start = 0);

    //宽符串转换double
    //\@ src    字符串
    //\@ start  开始位置（默认从0开始）
    //\@ base   进制（默认十进制）
    static double as_double(const wchar_t* src, std::size_t *start = 0);

    //字符串长度
    static std::size_t length(const wchar_t* buf);

    //是否是空串
    static bool empty(const wchar_t* buf);

    //是否相等
    static bool compare(const wchar_t* s1, const wchar_t* s2);
#pragma endregion

// 移植代码

// 如果是空
	static bool IsSpace(char in);

	static bool IsSpaceOrNewLine(char in);

	// 如果是行末
	static bool IsLineEnd(char in);

	/** @brief  Will perform a simple tokenize.
	*  @param  str         String to tokenize.
	*  @param  tokens      Array with tokens, will be empty if no token was found.
	*  @param  delimiters  Delimiter for tokenize.
	*  @return Number of found token.
	*/
	template<class string_type>
	static unsigned int tokenize(const string_type& str, std::vector<string_type>& tokens, const string_type& delimiters)
	{
		// Skip delimiters at beginning.
		typename string_type::size_type lastPos = str.find_first_not_of(delimiters, 0);

		// Find first "non-delimiter".
		typename string_type::size_type pos = str.find_first_of(delimiters, lastPos);
		while (string_type::npos != pos || string_type::npos != lastPos)
		{
			// Found a token, add it to the vector.
			string_type tmp = str.substr(lastPos, pos - lastPos);
			if (!tmp.empty() && ' ' != tmp[0])
				tokens.push_back(tmp);

			// Skip delimiters.  Note the "not_of"
			lastPos = str.find_first_not_of(delimiters, pos);

			// Find next "non-delimiter"
			pos = str.find_first_of(delimiters, lastPos);
		}

		return static_cast<unsigned int>(tokens.size());
	}

	/** @brief Helper function to do platform independent string comparison.
	*
	*  This is required since strincmp() is not consistently available on
	*  all platforms. Some platforms use the '_' prefix, others don't even
	*  have such a function.
	*
	*  @param s1 First input string
	*  @param s2 Second input string
	*  @param n Macimum number of characters to compare
	*  @return 0 if the given strings are identical
	*/
	static int strincmp(const char *s1, const char *s2, unsigned int n);
};
#endif//_STX_UTIL_STRING_H