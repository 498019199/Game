#ifndef _STX_IOTYPE_H_
#define _STX_IOTYPE_H_
#pragma once
#include "../Container/macro.h"

typedef char    zint8_t;
typedef short   zint16_t;
typedef int     zint32_t;
//#if STX_PLATFORM_WIN32
    typedef long long int int64_t;
//#endif

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
#if STX_PLATFORM_WIN64
    typedef unsigned  __int64 uint64_t;
    typedef uint64_t  zsize_t;
#else
    typedef unsigned __int64 uint64_t;
    typedef unsigned long  zsize_t;
#endif//STX_PLATFORM_WIN64

    

enum ValueType
{
    TYPE_NONE = -1,
    CType_bool = 0,              //bool
    CType_int = 1,               //int
    CType_float = 2,             //float
    CType_double = 3,            //double
    CType_int64 = 4,             //int64_t
    CType_string = 5,            //char*
    CType_widestring = 6,           //wchar_t*
    CType_pointer = 7,           //void*
    CType_object = 8,            //persisid
    CType_buffer = 9,     //use define data
	CType_float2 = 10,
	CType_float3 = 11,
	CType_float4 = 12,
	CType_float4x4 = 13,
};

#define  ERROR_LogManager(msg, args) assert(msg);
//typedef (int)(const char* szEvent, IVarList& args);
#endif//_STX_IOTYPE_H_