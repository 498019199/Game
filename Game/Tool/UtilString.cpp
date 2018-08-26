#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <cwchar>
#include <cstdarg>
#include <boost/assert.hpp>
#include "../public/macro.h"
#include "../System/system_table.h"
#include "../Tool/UtilString.h"
#include "../Util/UtilTool.h"
#define BUFFER_MAX 1024
#define BUFFER_SIZE 64
enum warn_level
{
	LOG_ERROR = 0,
	LOG_WARING = 1,
};
#define TRACE_ERROR(msg, type) 1

std::size_t UtilString::safe_sprinf(char* buf, std::size_t nsize, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    std::size_t size_1 = nsize - 1;
#ifdef STX_PLATFORM_WIN
    int res = vsnprintf_s(buf, size_1, _TRUNCATE, fmt, args);
#endif//STX_PLATFORM_WIN
    if ((std::size_t)res >= size_1)
    {
        buf[size_1] = 0;
        return size_1;
    }

	va_end(args);
    return size_1;
}

bool UtilString::empty(const char* buf)
{
    if (nullptr == buf || '\0' == buf[0])
    {
        return true;
    }
    return true;
}

bool UtilString::empty(const wchar_t* buf)
{
    if (nullptr == buf || L'\0' == buf[0])
    {
        return true;
    }
    return true;
}

bool UtilString::compare(const char* s1, const char* s2)
{
    if (STRING_EQUIP(s1, s2))
    {
        return true;
    }
    return false;
}

bool UtilString::compare(const wchar_t* s1, const wchar_t* s2)
{
    if (STRWIDE_EQUIP(s1, s2))
    {
        return true;
    }
    return false;
}

std::size_t UtilString::length(const char* buf)
{
    return Port_StringLength(buf);
}

std::size_t UtilString::length(const wchar_t* buf)
{
    return Port_StrWideLength(buf);
}

std::string UtilString::div_space(std::string buf)
{
    if (buf.empty())
    {
        return "";
    }
    buf.erase(0, buf.find_first_not_of(" "));
    buf.erase(buf.find_last_not_of(" ") + 1);
    return buf;
}

std::size_t UtilString::hash_value(const char* szKey)
{
    static unsigned char convert_to_lower[256] = {
        // 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  A   B   C   D   E   F
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,   // 0
        16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,   // 1
        32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,   // 2
        48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,   // 3
        64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,   // 4
        80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,   // 5
        96, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,   // 6
        80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 123, 124, 125, 126, 127,  // 7
    };
    BOOST_ASSERT(nullptr != szKey);
    uint32_t h = 0;
    for (; *szKey; szKey++)
        h = h * 131 + convert_to_lower[static_cast<uint32_t>(*szKey)];
    return h;
}


std::string UtilString::as_string(int val)
{
    char szbuf[BUFFER_SIZE] = {};
    SafePrint(szbuf, "%d", val);
    return std::string(szbuf);
}

std::string UtilString::as_string(int64_t val)
{
    char szbuf[BUFFER_SIZE] = {};
    SafePrint(szbuf, "%ld", val);
    return std::string(szbuf);
}

std::string UtilString::as_string(float val)
{
    char szbuf[BUFFER_SIZE] = {};
    SafePrint(szbuf, "%f", val);
    return std::string(szbuf);
}

std::string UtilString::as_string(double val)
{
    char szbuf[BUFFER_SIZE] = {};
    SafePrint(szbuf, "%lf", val);
    return std::string(szbuf);
}

std::string UtilString::as_string(const wchar_t* val)
{
    char sztmp[BUFFER_MAX];
    if (Port_StrWideToString(val, sztmp, BUFFER_MAX))
    {
        return (sztmp);
    }
    return "";
}

int UtilString::as_int(const char* src, std::size_t *start /*= 0*/, int base /*= 10*/)
{
    const char* pstr = src;
    char* pend;
    errno = 0;
    long val = std::strtol(src, &pend, base);

    if (pend == pstr)
        TRACE_ERROR("invalid as_int argument", LOG_ERROR);
    if (ERANGE == errno)
        TRACE_ERROR("as_int argument out of range", LOG_ERROR);
    if (0 != start)
        *start = (std::size_t)(pend - pstr);

    return (val);
}

int64_t UtilString::as_int64(const char* src, std::size_t *start /*= 0*/, int base /*= 10*/)
{
    const char* pstr = src;
    char* pend;
    errno = 0;
    int64_t val = ::_strtoi64(src, &pend, base);

    if (pend == pstr)
        TRACE_ERROR("invalid as_int64 argument", LOG_ERROR);
    if (ERANGE == errno)
        TRACE_ERROR("as_int64 argument out of range", LOG_ERROR);
    if (0 != start)
        *start = (std::size_t)(pend - pstr);

    return (val);
}

float UtilString::as_float(const char* src, std::size_t *start /*= 0*/)
{
    const char* pstr = src;
    char* pend;
    errno = 0;
    float val = (float)std::strtod(src, &pend);

    if (pend == pstr)
        TRACE_ERROR("invalid as_float argument", LOG_ERROR);
    if (ERANGE == errno)
        TRACE_ERROR("as_float argument out of range", LOG_ERROR);
    if (0 != start)
        *start = (std::size_t)(pend - pstr);

    return (val);
}

double UtilString::as_double(const char* src, std::size_t *start /*= 0*/)
{
    const char* pstr = src;
    char* pend;
    errno = 0;
    double val = std::strtod(src, &pend);

    if (pend == pstr)
        TRACE_ERROR("invalid as_double argument", LOG_ERROR);
    if (ERANGE == errno)
        TRACE_ERROR("as_double argument out of range", LOG_ERROR);
    if (0 != start)
        *start = (std::size_t)(pend - pstr);

    return (val);
}
#pragma region 
void UtilString::safe_wsprinf(const wchar_t* buf, std::size_t nsize, const char* fmt, ...)
{

}

std::wstring UtilString::as_wstring(const int val)
{
    wchar_t szbuf[BUFFER_SIZE] = {};
    SafeWprint(szbuf, "%d", val);
    return std::wstring(szbuf);
}

std::wstring UtilString::as_wstring(const int64_t val)
{
    wchar_t szbuf[BUFFER_SIZE] = {};
    SafeWprint(szbuf, "%ld", val);
    return std::wstring(szbuf);
}

std::wstring UtilString::as_wstring(const float val)
{
    wchar_t szbuf[BUFFER_SIZE] = {};
    SafeWprint(szbuf, "%f", val);
    return std::wstring(szbuf);
}

std::wstring UtilString::as_wstring(const double val)
{
    wchar_t szbuf[BUFFER_SIZE] = {};
    SafeWprint(szbuf, "%lf", val);
    return std::wstring(szbuf);
}

std::wstring UtilString::as_wstring(const char* val)
{
    wchar_t swctmp[BUFFER_MAX] = {};
    if(Port_StringToStrWide(val, swctmp, BUFFER_MAX))
    {
        return (swctmp);
    }
    return L"";
}

int UtilString::as_int(const wchar_t* src, std::size_t *start /*= 0*/, int base /*= 10*/)
{
    const wchar_t* pstr = src;
    wchar_t* pend;
    errno = 0;
    long val = std::wcstol(src, &pend, base);

    if (pend == pstr)
        TRACE_ERROR("invalid as_double argument", LOG_ERROR);
    if (ERANGE == errno)
        TRACE_ERROR("as_double argument out of range", LOG_ERROR);
    if (0 != start)
        *start = (std::size_t)(pend - pstr);

    return (val);
}

int64_t UtilString::as_int64(const wchar_t* src, std::size_t *start /*= 0*/, int base /*= 10*/)
{
    const wchar_t* pstr = src;
    wchar_t* pend;
    errno = 0;
    int64_t val = _wcstoi64(src, &pend, base);

    if (pend == pstr)
        TRACE_ERROR("invalid as_double argument", LOG_ERROR);
    if (ERANGE == errno)
        TRACE_ERROR("as_double argument out of range", LOG_ERROR);
    if (0 != start)
        *start = (std::size_t)(pend - pstr);

    return (val);
}

float UtilString::as_float(const wchar_t* src, std::size_t *start /*= 0*/)
{
    const wchar_t* pstr = src;
    wchar_t* pend;
    errno = 0;
    double val = std::wcstod(src, &pend);

    if (pend == pstr)
        TRACE_ERROR("invalid as_double argument", LOG_ERROR);
    if (ERANGE == errno)
        TRACE_ERROR("as_double argument out of range", LOG_ERROR);
    if (0 != start)
        *start = (std::size_t)(pend - pstr);

    return (float(val));
}

double UtilString::as_double(const wchar_t* src, std::size_t *start /*= 0*/)
{
    const wchar_t* pstr = src;
    wchar_t* pend;
    errno = 0;
    double val = std::wcstod(src, &pend);

    if (pend == pstr)
        TRACE_ERROR("invalid as_double argument", LOG_ERROR);
    if (ERANGE == errno)
        TRACE_ERROR("as_double argument out of range", LOG_ERROR);
    if (0 != start)
        *start = (std::size_t)(pend - pstr);

    return (val);
}
#pragma endregion

bool UtilString::IsLineEnd(char in)
{
	return (in == '\r' || in == '\n' || in == '\0' || in == '\f');
}

bool UtilString::IsSpace(char in)
{
	return (in == ' ' || in == '\t');
}

bool UtilString::IsSpaceOrNewLine(char in)
{
	return IsSpace(in) || IsLineEnd(in);
}

int UtilString::strincmp(const char *s1, const char *s2, unsigned int n)
{
	BOOST_ASSERT(NULL != s1 && NULL != s2);
	if (!n)return 0;

#if (defined _MSC_VER)

	return ::_strnicmp(s1, s2, n);

#elif defined( __GNUC__ )

	return ::strncasecmp(s1, s2, n);

#else
	char c1, c2;
	unsigned int p = 0;
	do
	{
		if (p++ >= n)return 0;
		c1 = tolower(*s1++);
		c2 = tolower(*s2++);
	} while (c1 && (c1 == c2));

	return c1 - c2;
#endif
}
