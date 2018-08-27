// 移植kleyGE 2018年7月29日 固定长度数组 zhangbei

#ifndef _KFL_ARRAYREF_HPP
#define _KFL_ARRAYREF_HPP
#include "../Util/UtilTool.h"
#include <vector>
#include <boost/assert.hpp>

template <typename T>
class ArrayRef
{
public:
	typedef T const * iterator;
	typedef T const * const_iterator;
	typedef size_t size_type;

	typedef std::reverse_iterator<iterator> reverse_iterator;

public:
	constexpr ArrayRef()
		: m_pHead(nullptr), m_nSize(0)
	{
	}

	ArrayRef(ArrayRef const & rhs)
		: m_pHead(rhs.data()), m_nSize(rhs.size())
	{
	}

	constexpr ArrayRef(T const & t)
		: m_pHead(&t), m_nSize(1)
	{
	}

	constexpr ArrayRef(T const * data, size_t size)
		: m_pHead(data), m_nSize(size)
	{
	}

	constexpr ArrayRef(T const * begin, T const * end)
		: m_pHead(begin), m_nSize(end - begin)
	{
	}

	template <typename A>
	constexpr ArrayRef(std::vector<T, A> const & v)
		: m_pHead(v.data()), m_nSize(v.size())
	{
	}

	template <size_t N>
	constexpr ArrayRef(T const (&arr)[N])
		: m_pHead(arr), m_nSize(N)
	{
	}

	constexpr ArrayRef(std::initializer_list<T> const & v)
		: m_pHead(v.begin() == v.end() ? nullptr : v.begin()), m_nSize(v.size())
	{
	}

	template <typename U>
	constexpr ArrayRef(ArrayRef<U*> const & rhs,
		typename std::enable_if<std::is_convertible<U* const *, T const *>::value>::type* = 0)
		: m_pHead(rhs.data()), m_nSize(rhs.size())
	{
	}

	template <typename U, typename A>
	constexpr ArrayRef(std::vector<U*, A> const & v,
		typename std::enable_if<std::is_convertible<U* const *, T const *>::value>::type* = 0)
		: m_pHead(v.data()), m_nSize(v.size())
	{
	}

	constexpr iterator begin() const
	{
		return m_pHead;
	}
	constexpr iterator end() const
	{
		return m_pHead + m_nSize;
	}

	constexpr reverse_iterator rbegin() const
	{
		return reverse_iterator(this->end());
	}
	constexpr reverse_iterator rend() const
	{
		return reverse_iterator(this->begin());
	}

	constexpr T const * data() const
	{
		return m_pHead;
	}

	constexpr size_t size() const
	{
		return m_nSize;
	}

	constexpr bool empty() const
	{
		return m_nSize == 0;
	}

	constexpr T const & front() const
	{
		BOOST_ASSERT(!this->empty());
		return m_pHead[0];
	}

	constexpr T const & back() const
	{
		BOOST_ASSERT(!this->empty());
		return m_pHead[m_nSize - 1];
	}

	template <typename Alloc>
	ArrayRef<T> Copy(Alloc& alloc)
	{
		T* buff = alloc.template allocate<T>(m_nSize);
		std::uninitialized_copy(this->begin(), this->end(), buff);
		return ArrayRef<T>(buff, m_nSize);
	}

	constexpr ArrayRef<T> Slice(uint32_t n) const
	{
		BOOST_ASSERT_MSG(n <= this->size(), "Invalid specifier");
		return ArrayRef<T>(this->data() + n, this->size() - n);
	}

	constexpr ArrayRef<T> Slice(uint32_t n, uint32_t m) const
	{
		BOOST_ASSERT_MSG(n + m <= this->size(), "Invalid specifier");
		return ArrayRef<T>(this->data() + n, m);
	}

	constexpr ArrayRef<T> DropBack(uint32_t n = 1) const
	{
		BOOST_ASSERT_MSG(this->size() >= n, "Dropping more elements than exist");
		return this->Slice(0, this->Size() - n);
	}

	constexpr T const & operator[](size_t index) const
	{
		BOOST_ASSERT_MSG(index < m_nSize, "Invalid index!");
		return m_pHead[index];
	}

	constexpr std::vector<T> ToVector() const
	{
		return std::vector<T>(m_pHead, m_pHead + m_nSize);
	}

private:
	T const * m_pHead;
	size_type m_nSize;
};

template <typename T>
inline bool operator==(ArrayRef<T> lhs, ArrayRef<T> rhs)
{
	if (lhs.size() != rhs.size())
	{
		return false;
	}
	return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename T>
inline bool operator!=(ArrayRef<T> lhs, ArrayRef<T> rhs)
{
	return !(lhs == rhs);
}

template <typename T>
ArrayRef<T> MakeArrayRef(T const & t)
{
	return ArrayRef<T>(t);
}

template <typename T>
ArrayRef<T> MakeArrayRef(T const * data, size_t size)
{
	return ArrayRef<T>(data, size);
}

template <typename T>
ArrayRef<T> MakeArrayRef(T const * begin, T const * end)
{
	return ArrayRef<T>(begin, end);
}

template <typename T, typename A>
ArrayRef<T> MakeArrayRef(std::vector<T, A> const & v)
{
	return ArrayRef<T>(v);
}

template <typename T, size_t N>
ArrayRef<T> MakeArrayRef(T const (&arr)[N])
{
	return ArrayRef<T>(arr);
}

template <typename T>
ArrayRef<T> MakeArrayRef(std::initializer_list<T> const & v)
{
	return ArrayRef<T>(v);
}

#endif		// _KFL_ARRAYREF_HPP
