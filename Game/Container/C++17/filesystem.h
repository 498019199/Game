// 2018Äê8ÔÂ1ÈÕ
#ifndef _KFL_CXX17_FILESYSTEM_HPP
#define _KFL_CXX17_FILESYSTEM_HPP
#pragma once

#include "../Container/macro.h"

#if defined(STX_CXX17_LIBRARY_FILESYSTEM_SUPPORT)
#include <filesystem>
#elif defined(STX_TS_LIBRARY_FILESYSTEM_SUPPORT)
#include <experimental/filesystem>
namespace std
{
	namespace filesystem = experimental::filesystem;
}
#else
#include <boost/filesystem.hpp>
namespace std
{
	namespace filesystem = boost::filesystem;
}
#endif
#endif		// _KFL_CXX17_FILESYSTEM_HPP
