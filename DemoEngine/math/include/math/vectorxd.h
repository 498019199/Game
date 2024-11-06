#pragma once
#include <array>
#include <string>
#include <sstream>
#include "math/math.h"
#include "math_helper.h"

#ifdef PLATFORM_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable : 4244) // Conversion from doubel to int64
#pragma warning(pop)
#endif//PLATFORM_COMPILER_MSVC

namespace MathWorker
{

template <typename T, int N>
class Vector_T
{
    template<typename U, int M>
    friend class Vector_T;

	using DetailType = std::array<T, N>;
public:
    typedef typename DetailType::value_type value_type;
	using pointer = value_type* ;
	using const_pointer = value_type const * ;
	typedef typename DetailType::reference reference;
	typedef typename DetailType::const_reference const_reference;
	typedef typename DetailType::iterator iterator;
	typedef typename DetailType::const_iterator const_iterator;
	typedef typename DetailType::size_type size_type;
	typedef typename DetailType::difference_type difference_type;
	enum{ elem_num = N };

public:
	Vector_T()  noexcept
	{}
	
	explicit Vector_T(const T* rhs) noexcept
	{
    	MathHelper::vector_helper<T, N>
    	::DoCopy(data(), rhs);
	}

	explicit Vector_T(const T& rhs) noexcept
	{
    	MathHelper::vector_helper<T, N>
    	::DoAssign(data(), rhs);
	}

	Vector_T(const Vector_T& rhs) noexcept
	{
    	MathHelper::vector_helper<T, N>
    	::DoCopy(data(), rhs.data());
	}

	Vector_T(Vector_T&& rhs) noexcept
	    :vec_(std::move(rhs.vec_))
	{}

	template<typename U, int M>
	Vector_T(const Vector_T<U, M>& rhs) noexcept
	{
		static_assert(M >= N, "Could not convert to a smaller vector.");
		MathHelper::vector_helper<T, N>
			::DoCopy(data(), rhs.data());
	}

	Vector_T(const T& x, const T& y) noexcept
		:vec_ {{x, y}}
	{
		static_assert(2 == elem_num, "must be 2D vecotr");
	}
	Vector_T(T&& x, T&& y) noexcept
		: vec_{ {std::move(x), std::move(y)} }
	{
		static_assert(2 == elem_num, "must be 2D vecotr");
	}
	Vector_T(const T& x, const T& y, const T& z) noexcept
		: vec_{ {x, y , z} }
	{
		static_assert(3 == elem_num, "must be 3D vecotr");
	}
	Vector_T(T&& x, T&& y, T&& z) noexcept
		: vec_{ {std::move(x), std::move(y) , std::move(z)} }
	{
		static_assert(3 == elem_num, "must be 3D vecotr");
	}
	Vector_T(const T& x, const T& y, const T& z, const T& w) noexcept
		: vec_{ {x, y , z, w} }
	{
		static_assert(4 == elem_num, "must be 4D vecotr");
	}
	Vector_T(T&& x, T&& y, T&& z, T&& w) noexcept
		: vec_{ {std::move(x), std::move(y) , std::move(z), std::move(w)} }
	{
		static_assert(4 == elem_num, "must be 4D vecotr");
	}

	// size
	size_t size() noexcept
	{
		return elem_num;
	}

	size_t size() const noexcept
	{
		return elem_num;
	}

	// 零向量
	static Vector_T const Zero() noexcept
	{
		static Vector_T<T, N> const zero(value_type(0));
		return zero;
	}

	iterator begin() noexcept
	{
		return vec_.begin();
	}
	constexpr const_iterator begin() const noexcept
	{
		return vec_.begin();
	}
	iterator end() noexcept
	{
		return vec_.end();
	}
	constexpr const_iterator end() const noexcept
	{
		return vec_.end();
	}
	reference operator[](size_t off) noexcept
	{
		return vec_[off];
	}
	constexpr const_reference operator[](size_t off) const noexcept
	{
		return vec_[off];
	}
	reference x() noexcept
	{
		return vec_[0];
	}
	constexpr const_reference x() const noexcept
	{
		return vec_[0];
	}
	reference y() noexcept
	{
		static_assert(elem_num >= 2, "must be 2D vecotr");
		return vec_[1];
	}
	constexpr const_reference y() const noexcept
	{
		static_assert(elem_num >= 2, "must be 2D vecotr");
		return vec_[1];
	}
	reference z() noexcept
	{
		static_assert(elem_num >= 3, "must be 3D vecotr");
		return vec_[2];
	}
	constexpr const_reference z() const noexcept
	{
		static_assert(elem_num >= 3, "must be 3D vecotr");
		return vec_[2];
	}
	reference w() noexcept
	{
		static_assert(elem_num >= 4, "must be 4D vecotr");
		return vec_[3];
	}
	constexpr const_reference w() const noexcept
	{
		static_assert(elem_num >= 4, "must be 4D vecotr");
		return vec_[3];
	}
	constexpr pointer data() noexcept
	{
		return &vec_[0];
	}
	constexpr const_pointer data() const noexcept
	{
		return &vec_[0];
	}
	
	// operator +
	template <typename U>
	const Vector_T& operator+=(const Vector_T<U, N>& rhs) noexcept
	{
    	MathHelper::vector_helper<T, N>::DoAdd(data(), data(), rhs.data());
    	return *this;
	}

	template <typename U>
	const Vector_T& operator+=(const U& rhs) noexcept
	{
		MathHelper::vector_helper<T, N>::DoAdd(data(), data(), rhs);
		return *this;
	}

	// operator -
	template <typename U>
	const Vector_T& operator-=(const Vector_T<U, N>& rhs) noexcept
	{
		MathHelper::vector_helper<T, N>::DoSub(data(), data(), rhs.data());
		return *this;
	}

	template <typename U>
	const Vector_T& operator-=(const U& rhs) noexcept
	{
    	MathHelper::vector_helper<T, N>::DoSub(data(), data(), rhs);
    	return *this;
	}

	// operator 叉积
	Vector_T operator^(const Vector_T& rhs) const noexcept
	{
		return Cross(*this, rhs);
	}

	// operator 点积
	T operator|(const Vector_T& rhs) const noexcept
	{
		return Dot(*this, rhs);
	}

	// operator * 数乘
	template <typename U>
	const Vector_T& operator*=(const U& rhs) noexcept
	{
		MathHelper::vector_helper<T, N>::DoScale(data(), data(), rhs);
		return *this;
	}

	// operator /
	template <typename U>
	const Vector_T& operator/=(const Vector_T<U, N>& rhs) const noexcept
	{
		MathHelper::vector_helper<U, N>::DoDiv(data(), data(), rhs.data());
		return *this;
	}

	template <typename U>
	const Vector_T& operator/=(const U& rhs) noexcept
	{
    	MathHelper::vector_helper<T, N>::DoScale(data(), data(), rhs);
    	return *this;
	}

	// 复制构造
	Vector_T& operator=(const Vector_T& rhs) noexcept
	{
    	if (this != &rhs)
        	vec_ = rhs.vec_;
    	return *this;
	}

	Vector_T& operator=(Vector_T&& rhs) noexcept
	{
		vec_ = std::move(rhs.vec_);
		return *this;
	}

	template <typename U, int M>
	Vector_T& operator=(Vector_T<U, M> const & rhs) noexcept
	{
		static_assert(M >= N, "Could not assign to a smaller vector.");

		MathHelper::vector_helper<T, N>::DoCopy(data(), rhs.data());
		return *this;
	}

	// 正
	const Vector_T operator+() const  noexcept
	{
    	return *this;
	}

	// 负
	const Vector_T operator-() const noexcept
	{
		Vector_T tmp(*this);
		MathHelper::vector_helper<T, N>::DoNegate(tmp.data(), data());
		return tmp;
	}


	void swap(Vector_T& rhs) noexcept
	{
		MathHelper::vector_helper<T, N>::DoSwap(data(), rhs.data());
	}

	// operator ==
	bool operator==(const Vector_T& rhs) const noexcept
	{
		return MathHelper::vector_helper<T, N>::DoEquip(data(), rhs.data());
	}

	// operator ==
	bool operator!=(const Vector_T& rhs) const noexcept
	{
		return !(this->operator==(rhs));
	}

private:
	DetailType vec_;
};

template <typename T, int N>
Vector_T<T, N> operator+(const Vector_T<T, N>& lhs, const Vector_T<T, N>& rhs) noexcept
{
    return Vector_T<T, N>(lhs).operator+=(rhs);
}

template <typename T, int N>
Vector_T<T, N> operator-(const Vector_T<T, N>& lhs, const Vector_T<T, N>& rhs) noexcept
{
    return Vector_T<T, N>(lhs).operator-=(rhs);
}

template <typename T, int N, typename U>
Vector_T<T, N> operator*(const U& lhs, const Vector_T<T, N>& rhs) noexcept
{
    return Vector_T<T, N>(rhs).operator*=(lhs);
}

template <typename T, int N, typename U>
Vector_T<T, N> operator*(const Vector_T<T, N>& lhs, const U& rhs) noexcept
{
    return Vector_T<T, N>(lhs).operator*=(rhs);
}

template <typename T, int N, typename U>
Vector_T<T, N> operator/(const U& lhs, const Vector_T<T, N>& rhs) noexcept
{
    return Vector_T<T, N>(rhs).operator/=(lhs);
}

template <typename T, int N, typename U>
Vector_T<T, N> operator/(const Vector_T<T, N>& lhs, const U& rhs) noexcept
{
    return Vector_T<T, N>(lhs).operator/=(rhs);
}

// print
template <typename T, int N>
std::ostream& operator<<(std::ostream& os, const Vector_T<T, N>& vec)
{
	for (size_t i = 0; i < N; i++)
	{
		os << vec[i] << " ";
	}
	return os;
}
}
