#ifndef STX_WINDOWS_THREAD_H
#define STX_WINDOWS_THREAD_H
#pragma once
#include <windows.h>
typedef CRITICAL_SECTION pthread_t;

inline void Port_ThreadUnLock(pthread_t *pthread)
{
    LeaveCriticalSection(pthread);
}

inline void Port_ThreadLock(pthread_t *pthread)
{
    EnterCriticalSection(pthread);
}

inline void Port_CreateThread(pthread_t* pthread)
{
    InitializeCriticalSection(pthread);
}

inline void Port_DestroyTread(pthread_t* pthread)
{
    DeleteCriticalSection(pthread);
}
#endif//STX_WINDOWS_THREAD_H