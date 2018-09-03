#ifndef STXMATH_QUATERNATION_H
#define  STXMATH_QUATERNATION_H
#pragma once
#include <boost/operators.hpp>
#include "MathDefine.h"

template <typename T>
class Quaternion_T final : boost::addable<Quaternion_T<T>,
	boost::subtractable<Quaternion_T<T>,
	boost::dividable2<Quaternion_T<T>, T,
	boost::multipliable<Quaternion_T<T>,
	boost::multipliable2<Quaternion_T<T>, T,
	boost::equality_comparable<Quaternion_T<T>>>>>>>
{
public:
	enum { elem_num = 4 };
	typedef T value_type;
	typedef typename Vector_T<T, elem_num>::pointer pointer;
	typedef typename Vector_T<T, elem_num>::const_pointer const_pointer;
	typedef typename Vector_T<T, elem_num>::reference reference;
	typedef typename Vector_T<T, elem_num>::const_reference const_reference;
	typedef typename Vector_T<T, elem_num>::iterator iterator;
	typedef typename Vector_T<T, elem_num>::const_iterator const_iterator;
	typedef typename Vector_T<T, elem_num>::size_type size_type;
public:
	constexpr Quaternion_T() noexcept
	{
	}

	explicit constexpr Quaternion_T(const T * rhs) noexcept;
	constexpr Quaternion_T(const Vector_T<T, 3>& vec, T s) noexcept;
	Quaternion_T(const Quaternion_T & rhs) noexcept;
	Quaternion_T(Quaternion_T&& rhs) noexcept;
	constexpr Quaternion_T(T x, T y, T z, T w) noexcept;

	// 取向量
	iterator begin() noexcept
	{
		return quat.begin();
	}
	constexpr const_iterator begin() const noexcept
	{
		return quat.begin();
	}
	iterator end() noexcept
	{
		return quat.end();
	}
	constexpr const_iterator end() const noexcept
	{
		return quat.end();
	}
	reference operator[](size_type nIndex) noexcept
	{
		return quat[nIndex];
	}
	constexpr const_reference operator[](size_type nIndex) const noexcept
	{
		return quat[nIndex];
	}

	reference x() noexcept
	{
		return quat[0];
	}
	constexpr const_reference x() const noexcept
	{
		return quat[0];
	}
	reference y() noexcept
	{
		return quat[1];
	}
	constexpr const_reference y() const noexcept
	{
		return quat[1];
	}
	reference z() noexcept
	{
		return quat[2];
	}
	constexpr const_reference z() const noexcept
	{
		return quat[2];
	}
	reference w() noexcept
	{
		return quat[3];
	}
	constexpr const_reference w() const noexcept
	{
		return quat[3];
	}

	// 赋值操作符
	const Quaternion_T& operator+=(const Quaternion_T & rhs) noexcept;
	const Quaternion_T& operator-=(const Quaternion_T & rhs) noexcept;
	const Quaternion_T& operator*=(const Quaternion_T & rhs) noexcept;
	const Quaternion_T& operator*=(T rhs) noexcept;
	const Quaternion_T& operator/=(T rhs) noexcept;
	Quaternion_T& operator=(const Quaternion_T & rhs) noexcept;
	Quaternion_T& operator=(Quaternion_T&& rhs) noexcept;

	// 一元操作符
	Quaternion_T const operator+() const noexcept;
	Quaternion_T const operator-() const noexcept;

	// 取方向向量
	const Vector_T<T, 3> GetV() const noexcept;
	void SetV(Vector_T<T, 3> const & rhs) noexcept;

	bool operator==(Quaternion_T<T> const & rhs) const noexcept;
	static const Quaternion_T& Identity() noexcept;
private:
	Vector_T<T, elem_num> quat;
};
#endif //STXMATH_QUATERNATION_H