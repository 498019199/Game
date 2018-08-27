/**
 * @file DllLoader.hpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KFL, a subproject of KlayGE
 * For the latest info, see http://www.klayge.org
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You may alternatively use this source under the terms of
 * the KlayGE Proprietary License (KPL). You can obtained such a license
 * from http://www.klayge.org/licensing/.
 */

#ifndef _KFL_DLLLOADER_HPP
#define _KFL_DLLLOADER_HPP

#pragma once

#include <string>
#include "../Container/macro.h"
#if defined(KLAYGE_COMPILER_MSVC) || defined(STX_COMPILER_CLANGC2)
	#define DLL_PREFIX ""
#else
	#define DLL_PREFIX "lib"
#endif
#if defined(STX_PLATFORM_WIN)
	#define DLL_EXT_NAME "dll"
#else
	#define DLL_EXT_NAME "so"
#endif

#ifdef _DEBUG
#define STX_DBG_SUFFIX "_d"
#else
#define STX_DBG_SUFFIX ""
#endif

#define STX_COMPILER_NAME clang
#define STX_OUTPUT_SUFFIX "_" STX_STRINGIZE(STX_COMPILER_NAME) STX_STRINGIZE(STX_COMPILER_VERSION) STX_DBG_SUFFIX
#define DLL_SUFFIX STX_OUTPUT_SUFFIX "." DLL_EXT_NAME

class DllLoader
{
public:
	DllLoader();
	~DllLoader();

	bool Load(const std::string& dll_name);
	void Free();

	void* GetProcAddress(const std::string & proc_name);

private:
	void* pHandleDLL;
};

#endif		// _KFL_DLLLOADER_HPP
