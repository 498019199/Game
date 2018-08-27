#ifndef _KFL_HASH_HPP
#define _KFL_HASH_HPP
#pragma once
#define PRIME_NUM 0x9e3779b9
#define KLAYGE_COMPILER_MSVC

#ifdef KLAYGE_COMPILER_MSVC
	#pragma warning(disable: 4307) // The hash here could cause integral constant overflow
#endif

size_t constexpr CTHashImpl(char const * str, size_t seed)
{
	return 0 == *str ? seed : CTHashImpl(str + 1, seed ^ (*str + PRIME_NUM + (seed << 6) + (seed >> 2)));
}

#ifdef KLAYGE_COMPILER_MSVC
	template <size_t N>
	struct EnsureConst
	{
		static size_t constexpr value = N;
	};
	#define CT_HASH(x) (EnsureConst<CTHashImpl(x, 0)>::value)
#else
	#define CT_HASH(x) (CTHashImpl(x, 0))
#endif

template <typename SizeT>
inline void HashCombineImpl(SizeT& seed, SizeT value)
{
	seed ^= value + PRIME_NUM + (seed << 6) + (seed >> 2);
}

inline size_t RT_HASH(char const * str)
{
	size_t seed = 0;
	while (*str != 0)
	{
		HashCombineImpl(seed, static_cast<size_t>(*str));
		++str;
	}
	return seed;
}

#undef PRIME_NUM

template <typename T>
inline size_t HashValue(T v)
{
	return static_cast<size_t>(v);
}

template <typename T>
inline size_t HashValue(T* v)
{
	return static_cast<size_t>(reinterpret_cast<ptrdiff_t>(v));
}

template <typename T>
inline void HashCombine(size_t& seed, T const & v)
{
	return HashCombineImpl(seed, HashValue(v));
}

template <typename T>
inline void HashRange(size_t& seed, T first, T last)
{
	for (; first != last; ++first)
	{
		HashCombine(seed, *first);
	}
}

template <typename T>
inline size_t HashRange(T first, T last)
{
	size_t seed = 0;
	HashRange(seed, first, last);
	return seed;
}

#endif //_KFL_HASH_HPP
