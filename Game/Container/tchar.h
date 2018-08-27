#ifndef _STX_TCHAR_H_
#define _STX_TCHAR_H_
#pragma once
#include <cwchar>
#include <cassert>
#include "../Util/UtilTool.h"

template<typename Ty>
struct Trains
{
};

template<>
struct Trains <char>
{
    static bool compare(const char* s1, const char* s2)
    {
        return STRING_EQUIP(s1, s2);
    }

    static uint32_t length(const char* src)
    {
        return strlen(src);
    }

    static uint32_t hash_value(const char* szKey)
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
        assert(nullptr != szKey);
        uint32_t h = 0;
        for (; *szKey; szKey++)
            h = h * 131 + convert_to_lower[static_cast<uint32_t>(*szKey)];
        return h;
    }

    static bool equip(const char* s1, const char* s2)
    {
        if (hash_value(s1) == hash_value(s2))
            return true;
        return false;
    }

    static void copy(char* des, const char* src)
    {
        /*errno_t b = */STX_STRCPY(des, src);
    }

    static void put(char* des, char val)
    {
        *des = val;
    }

    static char* emptystr()
    {
        return "";
    }
};

template<>
struct Trains<wchar_t>
{
    static bool compare(const wchar_t* s1, const wchar_t* s2)
    {
        return STRWIDE_EQUIP(s1, s2);
    }

    static uint32_t length(const wchar_t* src)
    {
        return wcslen(src);
    }

    static void copy(wchar_t* des, const wchar_t* src)
    {
        STX_WCSCPY(des, src);
    }

    static uint32_t hash_value(const wchar_t* szKey)
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
            128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,  // 8
            144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,  // 9
            160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,  // A
            176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,  // B
            192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,  // C
            208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,  // D
            224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,  // E
            240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255   // F
        };
        assert(nullptr != szKey);
        uint32_t h = 0;
        for (; *szKey; szKey++)
            h = h * 131 + convert_to_lower[static_cast<uint32_t>(*szKey)];
        return h;
    }

    static bool equip(const wchar_t* s1, const wchar_t* s2)
    {
        if (hash_value(s1) == hash_value(s2))
            return true;
        return false;
    }

    static void put(wchar_t* des, wchar_t val)
    {
        *des = val;
    }

    static wchar_t* emptystr()
    {
        return L"";
    }
};
#endif//_STX_TCHAR_H_