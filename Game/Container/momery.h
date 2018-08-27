#ifndef _STX_MEMORY_H
#define _STX_MEMORY_H
#pragma once

#include <new>
#include <cassert>

#if _DEBUG  
    #include <stdlib.h>  
    #include <crtdbg.h>  
    #define NEW new( _CLIENT_BLOCK, __FILE__, __LINE__)
    //ÄÚ´æÐ¹Â©¼ì²â£¬ÔÚmainÌí¼Ó
#define MEMORY_CHECK_LEAKAGE _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#else  
    #define NEW new  
    #define MEMORY_CHECK_LEAKAGE
#endif  
#endif//_STX_MEMORY_H