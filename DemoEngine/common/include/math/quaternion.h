#pragma once
#include "math/vectorxd.h"

namespace RenderWorker
{
template <typename T>
class Quaternion_T
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

	static constexpr size_t size() noexcept
	{
		return elem_num;
	}

	// ȡ    
	iterator begin() noexcept
	{
		return quat_.begin();
	}
	constexpr const_iterator begin() const noexcept
	{
		return quat_.begin();
	}
	iterator end() noexcept
	{
		return quat_.end();
	}
	constexpr const_iterator end() const noexcept
	{
		return quat_.end();
	}
	reference operator[](size_type nIndex) noexcept
	{
		return quat_[nIndex];
	}
	constexpr const_reference operator[](size_type nIndex) const noexcept
	{
		return quat_[nIndex];
	}

	reference x() noexcept
	{
		return quat_[0];
	}
	constexpr const_reference x() const noexcept
	{
		return quat_[0];
	}
	reference y() noexcept
	{
		return quat_[1];
	}
	constexpr const_reference y() const noexcept
	{
		return quat_[1];
	}
	reference z() noexcept
	{
		return quat_[2];
	}
	constexpr const_reference z() const noexcept
	{
		return quat_[2];
	}
	reference w() noexcept
	{
		return quat_[3];
	}
	constexpr const_reference w() const noexcept
	{
		return quat_[3];
	}

	//   ֵ      
	const Quaternion_T& operator+=(const Quaternion_T & rhs) noexcept;
	const Quaternion_T& operator-=(const Quaternion_T & rhs) noexcept;
	const Quaternion_T& operator*=(const Quaternion_T & rhs) noexcept;
	const Quaternion_T& operator*=(T rhs) noexcept;
	const Quaternion_T& operator/=(T rhs) noexcept;
	Quaternion_T& operator=(const Quaternion_T & rhs) noexcept;
	Quaternion_T& operator=(Quaternion_T&& rhs) noexcept;

	Quaternion_T operator+(const Quaternion_T & rhs) const noexcept;
	Quaternion_T operator-(const Quaternion_T & rhs) const noexcept;
	Quaternion_T operator*(const Quaternion_T & rhs) const noexcept;
	Quaternion_T operator*(T rhs) const noexcept;
	Quaternion_T operator/(T rhs) const noexcept;
   
	Quaternion_T const operator+() const noexcept;
	Quaternion_T const operator-() const noexcept;
     
	// 取方向向量
	constexpr Vector_T<T, 3> const& v() const noexcept
	{
		return quat_.template AsVector<3>();
	}
	void v(Vector_T<T, 3> const & rhs) noexcept;

	constexpr Vector_T<T, 4> const& AsVector4() const noexcept
	{
		return quat_;
	}


	bool operator==(Quaternion_T<T> const & rhs) const noexcept;
	bool operator!=(Quaternion_T<T> const & rhs) const noexcept;
	static const Quaternion_T& Identity() noexcept;

	// print
	template <typename U>
	friend std::ostream& operator<<(std::ostream& os, const Quaternion_T<U>& rhs);
private:
	Vector_T<T, elem_num> quat_;
};

template <typename U>
std::ostream& operator<<(std::ostream& os, const Quaternion_T<U>& rhs)
{
	return os << rhs.quat_;
}

template <typename T>
Quaternion_T<T> operator+(const Quaternion_T<T>& lhs, const Quaternion_T<T>& rhs) noexcept
{
    return Quaternion_T<T>(lhs).operator+=(rhs);
}

template <typename T>
Quaternion_T<T> operator-(const Quaternion_T<T>& lhs, const Quaternion_T<T>& rhs) noexcept
{
    return Quaternion_T<T>(lhs).operator-=(rhs);
}

template <typename T>
Quaternion_T<T> operator*(const Quaternion_T<T>& lhs, const Quaternion_T<T>& rhs) noexcept
{
    return Quaternion_T<T>(lhs).operator*=(rhs);
}
template <typename T>
Quaternion_T<T> operator*(const Quaternion_T<T>& lhs, T rhs) noexcept
{
    return Quaternion_T<T>(lhs).operator*=(rhs);
}
template <typename T>
Quaternion_T<T> operator*(T lhs, const Quaternion_T<T>& rhs) noexcept
{
    return Quaternion_T<T>(rhs).operator*=(lhs);
}

template <typename T>
Quaternion_T<T> operator/(const Quaternion_T<T>& lhs, T rhs) noexcept
{
    return Quaternion_T<T>(lhs).operator/=(rhs);
}
using quater = Quaternion_T<float>;
}