#ifndef _STX_ITERATOR_H_
#define _STX_ITERATOR_H_
#pragma once

#if defined(STX_CXX17_LIBRARY_SIZE_AND_MORE_SUPPORT)
	#include <iterator>
#else
	namespace std
	{
		template <typename T>
		inline constexpr size_t size(T const & t)
		{
			return t.size();
		}

		template <typename T, size_t N>
		inline constexpr size_t size(T const (&)[N]) noexcept
		{
			return N;
		}
}
#endif
#endif		// _STX_ITERATOR_H_