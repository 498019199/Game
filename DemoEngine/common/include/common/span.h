#pragma once
#include <span>

namespace RenderWorker
{

template <typename ElementType>
constexpr std::span<ElementType> MakeSpan() noexcept
{
	return std::span<ElementType>();
}

template <typename ElementType>
constexpr std::span<ElementType> MakeSpan(ElementType* ptr, typename std::span<ElementType>::size_type count)
{
	return std::span<ElementType>(ptr, count);
}

template <typename ElementType>
constexpr std::span<ElementType> MakeSpan(ElementType* first_elem, ElementType* last_elem)
{
	return std::span<ElementType>(first_elem, last_elem);
}

template <typename ElementType, std::size_t N>
constexpr std::span<ElementType, N> MakeSpan(ElementType (&arr)[N]) noexcept
{
	return std::span<ElementType, N>(arr);
}

template <typename Container>
constexpr std::span<typename Container::value_type> MakeSpan(Container& cont)
{
	return std::span<typename Container::value_type>(cont);
}

template <typename Container>
constexpr std::span<typename Container::value_type const> MakeSpan(Container const& cont)
{
	return std::span<typename Container::value_type const>(cont);
}

template <typename Ptr>
constexpr std::span<typename Ptr::element_type> MakeSpan(Ptr& cont, std::size_t count)
{
	return std::span<typename Ptr::element_type>(cont, count);
}

template <typename Ptr>
constexpr std::span<typename Ptr::element_type> MakeSpan(Ptr& cont)
{
	return std::span<typename Ptr::element_type>(cont);
}

template <typename T>
constexpr std::span<T> MakeSpan(std::initializer_list<T> v)
{
	return std::span<T>(v.begin(), v.end());
}

template <typename T>
constexpr std::span<T const> MakeSpan(std::initializer_list<T const> v)
{
	return std::span<T const>(v.begin(), v.end());
}

template <int dummy, typename T>
constexpr std::span<T, 1> MakeSpan(T& val)
{
	return std::span<T, 1>(&val, 1);
}

template <int dummy, typename T>
constexpr std::span<T const, 1> MakeSpan(T const& val)
{
	return std::span<T const, 1>(&val, 1);
}

template <typename ElementType1, typename ElementType2, std::size_t FirstExtent, std::size_t SecondExtent>
constexpr bool operator==(std::span<ElementType1, FirstExtent> lhs, std::span<ElementType2, SecondExtent> rhs) noexcept
{
	return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename ElementType1, typename ElementType2, std::size_t Extent>
constexpr bool operator!=(std::span<ElementType1, Extent> lhs, std::span<ElementType2, Extent> rhs) noexcept
{
	return !(lhs == rhs);
}

template <typename ElementType1, typename ElementType2, std::size_t Extent>
constexpr bool operator<(std::span<ElementType1, Extent> lhs, std::span<ElementType2, Extent> rhs) noexcept
{
	return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename ElementType1, typename ElementType2, std::size_t Extent>
constexpr bool operator<=(std::span<ElementType1, Extent> lhs, std::span<ElementType2, Extent> rhs) noexcept
{
	return !(lhs > rhs);
}

template <typename ElementType1, typename ElementType2, std::size_t Extent>
constexpr bool operator>(std::span<ElementType1, Extent> lhs, std::span<ElementType2, Extent> rhs) noexcept
{
	return rhs < lhs;
}

template <typename ElementType1, typename ElementType2, std::size_t Extent>
constexpr bool operator>=(std::span<ElementType1, Extent> l, std::span<ElementType2, Extent> r) noexcept
{
	return !(l < r);
}
}