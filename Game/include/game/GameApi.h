#pragma once

#include <common/macro.h>

#ifndef GAME_API
#if defined(GAME_EXPORTS)
#define GAME_API ZENGINE_SYMBOL_EXPORT
#else
#define GAME_API ZENGINE_SYMBOL_IMPORT
#endif
#endif
