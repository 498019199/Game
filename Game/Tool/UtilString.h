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
    //�ַ���ת��int
    //\@ src    �ַ���
    //\@ start  ��ʼλ�ã�Ĭ�ϴ�0��ʼ��
    //\@ base   ���ƣ�Ĭ��ʮ���ƣ�
    static int as_int(const char* src, std::size_t *start = 0, int base = 10);

    //�ַ���ת��int64
    //\@ src    �ַ���
    //\@ start  ��ʼλ�ã�Ĭ�ϴ�0��ʼ��
    //\@ base   ���ƣ�Ĭ��ʮ���ƣ�
    static int64_t as_int64(const char* src, std::size_t *start = 0, int base = 10);

    //�ַ���ת��float
    //\@ src    �ַ���
    //\@ start  ��ʼλ�ã�Ĭ�ϴ�0��ʼ��
    //\@ base   ���ƣ�Ĭ��ʮ���ƣ�
    static float as_float(const char* src, std::size_t *start = 0);

    //�ַ���ת��double
    //\@ src    �ַ���
    //\@ start  ��ʼλ�ã�Ĭ�ϴ�0��ʼ��
    //\@ base   ���ƣ�Ĭ��ʮ���ƣ�
    static double as_double(const char* src, std::size_t *start = 0);

    //intת���ַ���
    //\@ val    ֵ
    static STD string as_string(int val);

    //int64_tת���ַ���
    //\@ val    ֵ
    static STD string as_string(int64_t val);
    
    //floatת���ַ���
    //\@ val    ֵ
    static STD string as_string(float val);
    
    //doubleת���ַ���
    //\@ val    ֵ
    static STD string as_string(double val);
    
    //���ַ�ת���ַ���
    //\@ val    ֵ
    static STD string as_string(const wchar_t* val);

    //���ֻ�������ƴ��
    //\@ buf    �洢�Ļ�������
    //\@ nsize  �洢��С
    //\@ fmt    ƴ�ӵĸ�ʽ��%d��
    //\@ ...    
    static std::size_t safe_sprinf(char* buf, std::size_t nsize, const char* fmt, ...);
    
    //�Ƿ����
    static bool compare(const char* s1, const char* s2);

    //�Ƿ��ǿմ�
    static bool empty(const char* buf);

    //�ַ�������
    static std::size_t length(const char* buf);

    //��ȡ��ϣֵ
    static std::size_t hash_value(const char* szKey);

    //�ַ���ǰ��ȥ�ո�
    static STD string div_space(STD string buf);

    //�ַ���ת���ַ�
    //ansiתutf8
    //utf8תutf16
    //utf16תutf8
#pragma region 
    //���ֻ�������ƴ��
    //\@ buf    �洢�Ļ�������
    //\@ nsize  �洢��С
    //\@ fmt    ƴ�ӵĸ�ʽ��%d��
    //\@ ...    
    static void safe_wsprinf(const wchar_t* buf, std::size_t nsize, const char* fmt, ...);

    //intת�������
    //\@ val    ֵ
    static STD wstring as_wstring(const int val);

    //int64_tת�������
    //\@ val    ֵ
    static STD wstring as_wstring(const int64_t val);

    //floatת�������
    //\@ val    ֵ
    static STD wstring as_wstring(const float val);

    //doubleת�������
    //\@ val    ֵ
    static STD wstring as_wstring(const double val);

    //�ַ���ת�������
    //\@ val    ֵ
    static STD wstring as_wstring(const char* val);

    //�����ת��int
    //\@ src    �ַ���
    //\@ start  ��ʼλ�ã�Ĭ�ϴ�0��ʼ��
    //\@ base   ���ƣ�Ĭ��ʮ���ƣ�
    static int as_int(const wchar_t* src, std::size_t *start = 0, int base = 10);

    //�����ת��int64
    //\@ src    �ַ���
    //\@ start  ��ʼλ�ã�Ĭ�ϴ�0��ʼ��
    //\@ base   ���ƣ�Ĭ��ʮ���ƣ�
    static int64_t as_int64(const wchar_t* src, std::size_t *start = 0, int base = 10);

    //�����ת��float
    //\@ src    �ַ���
    //\@ start  ��ʼλ�ã�Ĭ�ϴ�0��ʼ��
    //\@ base   ���ƣ�Ĭ��ʮ���ƣ�
    static float as_float(const wchar_t* src, std::size_t *start = 0);

    //�����ת��double
    //\@ src    �ַ���
    //\@ start  ��ʼλ�ã�Ĭ�ϴ�0��ʼ��
    //\@ base   ���ƣ�Ĭ��ʮ���ƣ�
    static double as_double(const wchar_t* src, std::size_t *start = 0);

    //�ַ�������
    static std::size_t length(const wchar_t* buf);

    //�Ƿ��ǿմ�
    static bool empty(const wchar_t* buf);

    //�Ƿ����
    static bool compare(const wchar_t* s1, const wchar_t* s2);
#pragma endregion

// ��ֲ����

// ����ǿ�
	static bool IsSpace(char in);

	static bool IsSpaceOrNewLine(char in);

	// �������ĩ
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