#ifndef STX_MATH_VETOR
#define STX_MATH_VETOR
#pragma once
#include <array>
#include <boost/operators.hpp>
#include "math_helper.h"

template <typename ValTy, int SIZE>
class Vector_T final : boost::addable<Vector_T<ValTy, SIZE>,
	boost::subtractable<Vector_T<ValTy, SIZE>,
	boost::multipliable<Vector_T<ValTy, SIZE>,
	boost::dividable<Vector_T<ValTy, SIZE>,
	boost::dividable2<Vector_T<ValTy, SIZE>, ValTy,
	boost::multipliable2<Vector_T<ValTy, SIZE>, ValTy,
	boost::addable2<Vector_T<ValTy, SIZE>, ValTy,
	boost::subtractable2<Vector_T<ValTy, SIZE>, ValTy,
	boost::equality_comparable<Vector_T<ValTy, SIZE>>>>>>>>>>
{
    template<typename ValTy, int SIZE>
    friend class Vector_T;

	typedef std::array<ValTy, SIZE> DetailType;
public:
	typedef typename DetailType::value_type value_type;
	typedef value_type* pointer;
	typedef  value_type const * const_pointer;

	typedef typename DetailType::reference reference;
	typedef typename DetailType::const_reference const_reference;

	typedef typename DetailType::iterator iterator;
	typedef typename DetailType::const_iterator const_iterator;

	typedef typename DetailType::size_type size_type;
	typedef typename DetailType::difference_type difference_type;

	enum{ elem_num = SIZE };
public:
	Vector_T()  noexcept
	{}
	explicit Vector_T(const ValTy* rhs) noexcept
	{
		MathHelper::vector_helper<ValTy, SIZE>
		::DoCopy(&vec[0], rhs);
	}
	explicit Vector_T(const ValTy& rhs) noexcept
	{
		MathHelper::vector_helper<ValTy, SIZE>
		::DoAssign(&vec[0], rhs);
	}
	Vector_T(const Vector_T& rhs) noexcept 
	{
		MathHelper::vector_helper<ValTy, SIZE>
		::DoCopy(&vec[0], &rhs.vec[0]);
	}
	Vector_T(Vector_T&& rhs) noexcept
		:vec(std::move(rhs.vec))
	{}

	template<typename U, int M>
	Vector_T(const Vector_T<U, M>& rhs) noexcept
	{
		static_assert(M >= SIZE, "Could not convert to a smaller vector.");

		MathHelper::vector_helper<ValTy, SIZE>
			::DoCopy(&vec[0], &rhs.vec[0]);
	}
	Vector_T(const ValTy& x, const ValTy& y) noexcept
		:vec {x, y}
	{
		static_assert(2 == elem_num, "must be 2D vecotr");
	}
	Vector_T(ValTy&& x, ValTy&& y) noexcept
		: vec{ std::move(x), std::move(y) }
	{
		static_assert(2 == elem_num, "must be 2D vecotr");
	}
	Vector_T(const ValTy& x, const ValTy& y, const ValTy& z) noexcept
		: vec{ x, y , z}
	{
		static_assert(3 == elem_num, "must be 3D vecotr");
	}
	Vector_T(ValTy&& x, ValTy&& y, ValTy&& z) noexcept
		: vec{ std::move(x), std::move(y) , std::move(z) }
	{
		static_assert(3 == elem_num, "must be 3D vecotr");
	}
	Vector_T(const ValTy& x, const ValTy& y, const ValTy& z, const ValTy& w) noexcept
		: vec{ x, y , z, w }
	{
		static_assert(4 == elem_num, "must be 4D vecotr");
	}
	Vector_T(ValTy&& x, ValTy&& y, ValTy&& z, ValTy&& w) noexcept
		: vec{ std::move(x), std::move(y) , std::move(z), std::move(w) }
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
	static Vector_T const zero() noexcept
	{
		static Vector_T<ValTy, SIZE> const zero(value_type(0));
		return zero;
	}

	iterator begin() noexcept
	{
		return vec.begin();
	}
	constexpr const_iterator begin() const noexcept
	{
		return vec.begin();
	}
	iterator end() noexcept
	{
		return vec.end();
	}
	constexpr const_iterator end() const noexcept
	{
		return vec.end();
	}
	reference operator[](size_type off) noexcept
	{
		return vec[off];
	}
	constexpr const_reference operator[](size_type off) const noexcept
	{
		return vec[off];
	}
	reference x() noexcept
	{
		return vec[0];
	}
	constexpr const_reference x() const noexcept
	{
		return vec[0];
	}
	reference y() noexcept
	{
		static_assert(elem_num >= 2, "must be 2D vecotr");
		return vec[1];
	}
	constexpr const_reference y() const noexcept
	{
		static_assert(elem_num >= 2, "must be 2D vecotr");
		return vec[1];
	}
	reference z() noexcept
	{
		static_assert(elem_num >= 3, "must be 3D vecotr");
		return vec[2];
	}
	constexpr const_reference z() const noexcept
	{
		static_assert(elem_num >= 3, "must be 3D vecotr");
		return vec[2];
	}
	reference w() noexcept
	{
		static_assert(elem_num >= 4, "must be 4D vecotr");
		return vec[3];
	}
	constexpr const_reference w() const noexcept
	{
		static_assert(elem_num >= 4, "must be 4D vecotr");
		return vec[3];
	}
		// operator +
	template <typename U>
	const Vector_T& operator+=(const Vector_T<U, SIZE>& rhs) noexcept
	{
		MathHelper::vector_helper<ValTy, SIZE>::DoAdd(&vec[0], &vec[0], &rhs.vec[0]);
		return *this;
	}
	template <typename U>
	const Vector_T& operator+=(const U& rhs) noexcept
	{
		MathHelper::vector_helper<ValTy, SIZE>::DoAdd(&vec[0], &vec[0], rhs);
		return *this;
	}
	// operator -
	template <typename U>
	const Vector_T& operator-=(const Vector_T<U, SIZE>& rhs) noexcept
	{
		MathHelper::vector_helper<U, SIZE>::DoSub(&vec[0], &vec[0], &rhs.vec[0]);
		return *this;
	}
	template <typename U>
	const Vector_T& operator-=(const U& rhs) noexcept
	{
		MathHelper::vector_helper<ValTy, SIZE>::DoSub(&vec[0], &vec[0], rhs);
		return *this;
	}
	// operator * 点积
	template <typename U>
	const Vector_T& operator*=(const Vector_T<U, SIZE>& rhs) noexcept
	{
		MathHelper::vector_helper<U, SIZE>::DoMul(&vec[0], &vec[0], &rhs.vec[0]);
		return *this;
	}
	// operator * 数乘
	template <typename U>
	const Vector_T& operator*=(const U& rhs) noexcept
	{
		MathHelper::vector_helper<ValTy, SIZE>::DoScale(&vec[0], &vec[0], rhs);
		return *this;
	}
	 // operator /
	template <typename U>
	const Vector_T& operator/=(const Vector_T<U, SIZE>& rhs) noexcept
	{
		MathHelper::vector_helper<U, SIZE>::DoDiv(&vec[0], &vec[0], &rhs.vec[0]);
		return *this;
	}
	template <typename U>
	const Vector_T& operator/=(const U& rhs) noexcept
	{
		MathHelper::vector_helper<ValTy, SIZE>::DoScale(&vec[0], &vec[0], rhs);
		return *this;
	}
		// 复制构造
	Vector_T& operator=(const Vector_T& rhs) noexcept
	{
		if (this != &rhs)
			vec = rhs.vec;
		return *this;
	}
	Vector_T& operator=(Vector_T&& rhs) noexcept
	{
		vec = std::move(rhs.vec);
		return *this;
	}
	template <typename U, int M>
	Vector_T& operator=(Vector_T<U, M> const & rhs) noexcept
	{
		static_assert(M >= SIZE, "Could not assign to a smaller vector.");

		MathHelper::vector_helper<ValTy, SIZE>::DoCopy(&vec[0], &rhs.vec[0]);
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
		MathHelper::vector_helper<ValTy, SIZE>::DoNegate(&tmp.vec[0], &this->vec[0]);
		return tmp;
	}
	void swap(Vector_T& rhs) noexcept
	{
		MathHelper::vector_helper<ValTy, SIZE>::DoSwap(&vec[0], &rhs[0]);
	}
	// operator ==
	bool operator==(const Vector_T& rhs) const noexcept
	{
		return MathHelper::vector_helper<ValTy, SIZE>::DoEquip(&vec[0], &rhs[0]);
	}
private:
	DetailType vec;
};
#endif//STX_MATH_VETOR

