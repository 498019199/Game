#ifndef ZB_SYSTEM_TABLE
#define ZB_SYSTEM_TABLE
#pragma once
#include "../Container/macro.h"

#ifdef STX_PLATFORM_WINDOWS
    #include "windows_socket.h"
    #include "PlatformUtil.h"
    #include "windows_thread.h"
#endif

#ifdef STX_PLATFORM_IOS
    #include "ios_file.h"
    #include "ios_socket.h"
#endif

#ifdef STX_PLATFORM_LINUX
    #include "linux_file.h"
    #include "linux_socket.h"
#endif

#ifdef STX_PLATFORM_WINDOWS
    #define  SPRINTF sprintf_s
typedef CRITICAL_SECTION pthread_t;
#endif
#ifdef STX_PLATFORM_LINUX
    #define  SPRINTF sprintf

#endif
#endif//ZB_SYSTEM_TABLE