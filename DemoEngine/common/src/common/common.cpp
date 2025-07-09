#include <common/common.h>

#ifdef ZENGINE_PLATFORM_WINDOWS
	#include <windows.h>
#else
	#include <cerrno>
	#include <cstdlib>
	#include <cwchar>
	#include <clocale>
#endif
#include <vector>
#include <algorithm>

namespace CommonWorker
{
// 把一个wstring转化为string
/////////////////////////////////////////////////////////////////////////////////
std::string& Convert(std::string& dest, std::wstring_view src)
{
#if defined ZENGINE_PLATFORM_WINDOWS
    int const mbs_len = WideCharToMultiByte(CP_ACP, 0, src.data(), static_cast<int>(src.size()), nullptr, 0, nullptr, nullptr);
    auto tmp = MakeUniquePtr<char[]>(mbs_len + 1);
    WideCharToMultiByte(CP_ACP, 0, src.data(), static_cast<int>(src.size()), tmp.get(), mbs_len, nullptr, nullptr);
#elif defined ZENGINE_PLATFORM_ANDROID
    // Hack for wcstombs
    std::vector<char> tmp;
    for (auto ch : src)
    {
        if (ch < 0x80)
        {
            tmp.push_back(static_cast<char>(ch));
        }
        else
        {
            tmp.push_back(static_cast<char>((ch >> 0) & 0xFF));
            tmp.push_back(static_cast<char>((ch >> 8) & 0xFF));
        }
    }
    tmp.push_back('\0');
    size_t const mbs_len = tmp.size() - 1;
#else
    std::setlocale(LC_CTYPE, "");
    size_t const mbs_len = wcstombs(nullptr, src.data(), src.size());
    auto tmp = MakeUniquePtr<char[]>(mbs_len + 1);
    wcstombs(tmp.get(), src.data(), mbs_len + 1);
#endif

    dest.assign(&tmp[0], &tmp[mbs_len]);
    return dest;
}

// 把一个string转化为string
/////////////////////////////////////////////////////////////////////////////////
std::string& Convert(std::string& dest, std::string_view src)
{
    dest = std::string(src);
    return dest;
}

// 把一个wstring转化为wstring
/////////////////////////////////////////////////////////////////////////////////
std::wstring& Convert(std::wstring& dest, std::wstring_view src)
{
    dest = std::wstring(src);
    return dest;
}

// 把一个string转化为wstring
/////////////////////////////////////////////////////////////////////////////////
std::wstring& Convert(std::wstring& dest, std::string_view src)
{
#if defined ZENGINE_PLATFORM_WINDOWS
    int const wcs_len = MultiByteToWideChar(CP_ACP, 0, src.data(), static_cast<int>(src.size()), nullptr, 0);
    auto tmp = MakeUniquePtr<wchar_t[]>(wcs_len + 1);
    MultiByteToWideChar(CP_ACP, 0, src.data(), static_cast<int>(src.size()), tmp.get(), wcs_len);
#elif defined ZENGINE_PLATFORM_ANDROID
    // Hack for mbstowcs
    std::vector<wchar_t> tmp;
    for (auto iter = src.begin(); iter != src.end(); ++ iter)
    {
        unsigned char ch = *iter;
        wchar_t wch = ch;
        if (ch >= 0x80)
        {
            ++ iter;
            if (iter != src.end())
            {
                wch |= (*iter) << 8;
            }
        }
        tmp.push_back(wch);
    }
    tmp.push_back(L'\0');
    size_t const wcs_len = tmp.size() - 1;
#else
    std::setlocale(LC_CTYPE, "");
    size_t const wcs_len = mbstowcs(nullptr, src.data(), src.size());
    auto tmp = MakeUniquePtr<wchar_t[]>(wcs_len + 1);
    mbstowcs(tmp.get(), src.data(), src.size());
#endif

    dest.assign(&tmp[0], &tmp[wcs_len]);

    return dest;
}
}