#ifndef ZB_WINDOS_STRING
#define ZB_WINDOS_STRING
#pragma once

#if defined(STX_PLATFORM_WIN)
#include <windows.h>
#endif

#include <assert.h>
#include "../Container/macro.h"
#include "../Container/var_type.h"

extern "C"
{
    //×Ö·û´®³¤¶È
    inline uint32_t PlatformStringLength(const char* src)
    {
#if defined(STX_PLATFORM_WIN)
		return MultiByteToWideChar(CP_ACP, 0, src, strlen(src), NULL, 0);
#elif defined (STX_PLATFORM_IOS)
#elif defined (STX_PLATFORM_LINUX)
#endif
    }


    //×Ö·û´®×ª¿í×Ö·û´®
    inline bool PlatformStringToStrWide(const char* src, wchar_t* buff, uint32_t len)
    {
#if defined(STX_PLATFORM_WIN)
        uint32_t min_size = (PlatformStringLength(src) + 1) * sizeof(wchar_t);
        if (len < min_size)
        {
            return false;
        }

        if (0 == MultiByteToWideChar(CP_ACP, 0, src, min_size, buff, len))
        {
            DWORD err = GetLastError();
        }
        return true;
#elif defined (STX_PLATFORM_IOS)
#elif defined (STX_PLATFORM_LINUX)
#endif
    }

    //¿í×Ö·û´®³¤¶È
    inline uint32_t PlatformStrWideLength(const wchar_t* src)
    {
#if defined(STX_PLATFORM_WIN)
        return WideCharToMultiByte(CP_OEMCP, NULL, src, -1, NULL, 0, NULL, FALSE);
#elif defined (STX_PLATFORM_IOS)
#elif defined (STX_PLATFORM_LINUX)
#endif
    }

    //¿í×Ö·û´®×ª×Ö·û´®
    inline bool PlatformStrWideToString(const wchar_t* src, char* buff, uint32_t len)
    {
#if defined(STX_PLATFORM_WIN)
        uint32_t min_size = PlatformStrWideLength(src);
        if (len < min_size)
        {
            return false;
        }

        if (0 == WideCharToMultiByte(CP_ACP, 0, src, -1, buff, len, NULL, false))
        {
            DWORD err = GetLastError();
        }
        return true;
#elif defined (STX_PLATFORM_IOS)
#elif defined (STX_PLATFORM_LINUX)
#endif
    }
};
#endif//ZB_WINDOS_STRING