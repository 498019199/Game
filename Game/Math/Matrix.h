#ifndef STXMATH_MATRIX_H
#define  STXMATH_MATRIX_H
#pragma once
#include "MathDefine.h"
#include "Vector.h"

#include <boost/operators.hpp>

template <typename Valty>
	class Matrix4_T final : boost::addable<Matrix4_T<Valty>,
										boost::subtractable<Matrix4_T<Valty>,
										boost::dividable2<Matrix4_T<Valty>, Valty,
										boost::multipliable2<Matrix4_T<Valty>, Valty,
										boost::multipliable<Matrix4_T<Valty>,
										boost::equality_comparable<Matrix4_T<Valty>>>>>>>
{
public:
	enum  { row_num = 4, col_num = 4};
	enum  { elem_num = row_num * col_num };

	typedef Valty					value_type;
	typedef Valty*					pointer;
	typedef const Valty*		const_pointer;
	typedef Valty&				reference;
	typedef const Valty&		const_reference;
	typedef Valty*					iterator;
	typedef const Valty*		const_iterator;
	typedef std::size_t			size_type;
	typedef std::size_t			difference_type;
public:
    Matrix4_T()
    {}
	explicit Matrix4_T(const Valty* rhs) noexcept;
	Matrix4_T(const Matrix4_T& rhs) noexcept;
    Matrix4_T(Matrix4_T&& rhs) noexcept;
    Matrix4_T(Valty f11, Valty f12, Valty f13, Valty f14,
                Valty f21, Valty f22, Valty f23, Valty f24,
                Valty f31, Valty f32, Valty f33, Valty f34,
                Valty f41, Valty f42, Valty f43, Valty f44) noexcept;
	
    reference operator()(size_type row, size_type col)noexcept
    {
        return m[row][col];
	}
	const_reference operator()(size_type row, size_type col) const noexcept
    {
        return m[row][col];
    }
	size_type size() noexcept
	{
		return elem_num;
	}
    size_type size() const noexcept
    {
        return elem_num;
    }
	iterator begin() noexcept
	{
		return &m[0][0];
	}
	constexpr const_iterator begin() const noexcept
	{
		return &m[0][0];
	}
	iterator end() noexcept
	{
		return begin() + elem_num;
	}
	constexpr const_iterator end() const noexcept
	{
		return begin() + elem_num;
	}
	reference operator[](size_t  off) noexcept
	{
        return *(begin() + off);
	}
	constexpr const_reference operator[](size_type  off)  const noexcept
    {
        return *(begin() + off);
    }
    static const Matrix4_T & Zero() noexcept;
    static const Matrix4_T & Identity() noexcept;
    void Row(size_type index, const Vector_T<Valty, 4>& rhs) noexcept;
    Vector_T<Valty, 4> Row(size_t index) const noexcept;
    void Col(size_type index, const Vector_T<Valty, 4>& rhs) noexcept;
    Vector_T<Valty, 4> Col(size_t index) const noexcept;

    //赋值操作符
    Matrix4_T& operator+=(const Matrix4_T& rhs) noexcept;
    Matrix4_T& operator-=(const Matrix4_T& rhs) noexcept;
    Matrix4_T& operator*=(const Matrix4_T& rhs) noexcept;
    Matrix4_T& operator*=(value_type rhs) noexcept;
    Matrix4_T& operator/=(value_type rhs) noexcept;

    Matrix4_T& operator=(const Matrix4_T& rhs) noexcept;
    Matrix4_T& operator=(Matrix4_T&& rhs) noexcept;

    //一元操作符
    Matrix4_T operator+() const noexcept;
    Matrix4_T operator-() const noexcept;

    bool operator==(const Matrix4_T& rhs) const noexcept;
private:
	Vector_T<Vector_T<Valty, col_num>, row_num> m;
};

#endif//STXMATH_MATRIX_H