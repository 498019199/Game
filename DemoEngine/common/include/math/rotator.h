#pragma once
#include "math/vectorxd.h"

namespace MathWorker
{
template <typename T>
class Rotator_T
{
public:
	enum { elem_num = 3 };
	typedef T value_type;

	typedef typename MathWorker::Vector_T<T, elem_num>::pointer pointer;
	typedef typename MathWorker::Vector_T<T, elem_num>::const_pointer const_pointer;
	typedef typename MathWorker::Vector_T<T, elem_num>::reference reference;
	typedef typename MathWorker::Vector_T<T, elem_num>::const_reference const_reference;
	typedef typename MathWorker:: Vector_T<T, elem_num>::iterator iterator;
	typedef typename MathWorker::Vector_T<T, elem_num>::const_iterator const_iterator;
	typedef typename MathWorker::Vector_T<T, elem_num>::size_type size_type;

public:
    constexpr Rotator_T() noexcept
    {}
    Rotator_T(const Rotator_T & rhs) noexcept;
	Rotator_T(Rotator_T&& rhs) noexcept;
    constexpr Rotator_T(T inF);
    constexpr Rotator_T(T in_x, T in_y, T in_z) noexcept;

	iterator begin() noexcept
	{
		return rot_.begin();
	}
	constexpr const_iterator begin() const noexcept
	{
		return rot_.begin();
	}
	iterator end() noexcept
	{
		return rot_.end();
	}
	constexpr const_iterator end() const noexcept
	{
		return rot_.end();
	}
	reference operator[](size_type nIndex) noexcept
	{
		return rot_[nIndex];
	}
	constexpr const_reference operator[](size_type nIndex) const noexcept
	{
		return rot_[nIndex];
	}

	reference pitch() noexcept
	{
		return rot_[0];
	}
	constexpr const_reference pitch() const noexcept
	{
		return rot_[0];
	}
	reference yaw() noexcept
	{
		return rot_[1];
	}
	constexpr const_reference yaw() const noexcept
	{
		return rot_[1];
	}
	reference roll() noexcept
	{
		return rot_[2];
	}
	constexpr const_reference roll() const noexcept
	{
		return rot_[2];
	}

    // 赋值操作符
	const Rotator_T& operator+=(const Rotator_T & rhs) noexcept;
	const Rotator_T& operator-=(const Rotator_T & rhs) noexcept;
	const Rotator_T& operator*=(const Rotator_T & rhs) noexcept;
	const Rotator_T& operator*=(T rhs) noexcept;
	const Rotator_T& operator/=(T rhs) noexcept;
	Rotator_T& operator=(const Rotator_T & rhs) noexcept;
	Rotator_T& operator=(Rotator_T&& rhs) noexcept;

	Rotator_T operator+(const Rotator_T & rhs) const noexcept;
	Rotator_T operator-(const Rotator_T & rhs) const noexcept;
	Rotator_T operator*(const Rotator_T & rhs) const noexcept;
	Rotator_T operator*(T rhs) const noexcept;
	Rotator_T operator/(T rhs) const noexcept;

	// 一元操作符
	Rotator_T const operator+() const noexcept;
	Rotator_T const operator-() const noexcept;

    bool operator==(Rotator_T<T> const & rhs) const noexcept;
	bool operator!=(Rotator_T<T> const & rhs) const noexcept;

    // print
	template <typename U>
	friend std::ostream& operator<<(std::ostream& os, const Rotator_T<U>& rhs);
private:
	Vector_T<T, elem_num> rot_;
};

template<typename U>
std::ostream& operator<<(std::ostream& os, const Rotator_T<U>& rhs)
{
    return os << rhs.rot_;
}

}