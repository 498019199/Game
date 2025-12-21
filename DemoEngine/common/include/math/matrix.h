#pragma once
#include <iostream>
#include "math/vectorxd.h"

namespace RenderWorker
{
template <typename T>
class Matrix4_T
{
public:
	enum  { row_num = 4, col_num = 4};
	enum  { elem_num = row_num * col_num };

	using value_type = T;
	using pointer = T*;
	using const_pointer = const T*;
	using reference = T&;
	using const_reference = const T&;
    using iterator = T*;
	using const_iterator = const T*;
    using size_type = std::size_t;
    using difference_type = std::size_t;
public:
    Matrix4_T()
    {}
	explicit Matrix4_T(const T* rhs) noexcept;
	Matrix4_T(const Matrix4_T& rhs) noexcept;
    Matrix4_T(Matrix4_T&& rhs) noexcept;
    Matrix4_T(T f11, T f12, T f13, T f14,
                T f21, T f22, T f23, T f24,
                T f31, T f32, T f33, T f34,
                T f41, T f42, T f43, T f44) noexcept;
	
    reference operator()(size_type row, size_type col)noexcept
    {
        return m_[row][col];
	}
	const_reference operator()(size_type row, size_type col) const noexcept
    {
        return m_[row][col];
    }
    static constexpr size_type size() noexcept
    {
        return elem_num;
    }
	iterator begin() noexcept
	{
		return &m_[0][0];
	}
	constexpr const_iterator begin() const noexcept
	{
		return &m_[0][0];
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
    constexpr pointer data() noexcept
    {
        return &m_[0][0];
    }
    constexpr const_pointer data() const noexcept
    {
        return &m_[0][0];
    }

    static const Matrix4_T & Zero() noexcept;
    static const Matrix4_T & Identity() noexcept;
    void Row(size_type index, const Vector_T<T, 4>& rhs) noexcept;
    const Vector_T<T, 4>& Row(size_t index) const noexcept;
    void Col(size_type index, const Vector_T<T, 4>& rhs) noexcept;
    Vector_T<T, 4> Col(size_t index) const noexcept;

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
    // print
    template <typename U>
    friend std::ostream& operator<<(std::ostream& os, const Matrix4_T<U>& rhs);

    friend Matrix4_T<T> operator+(const Matrix4_T<T>& lhs, const Matrix4_T<T>& rhs) noexcept
    {
		Matrix4_T<T> temp(lhs);
		return temp += rhs;
    }
    friend Matrix4_T<T> operator-(const Matrix4_T<T>& lhs, const Matrix4_T<T>& rhs) noexcept
    {
        Matrix4_T<T> temp(lhs);
        return temp -= rhs;
    }
    
    friend Matrix4_T<T> operator*(const Matrix4_T<T>& lhs, const Matrix4_T<T>& rhs) noexcept
    {
        Matrix4_T<T> temp(lhs);
        return temp *= rhs;
    }
    friend Matrix4_T<T> operator*(const Matrix4_T<T>& lhs, const T & rhs) noexcept
    {
        Matrix4_T<T> temp(lhs);
        return temp *= lhs;
    }
    friend Matrix4_T<T> operator*(const T & lhs, const Matrix4_T<T>& rhs) noexcept
    {
        return rhs * lhs;
    }

    friend Matrix4_T<T> operator/(const Matrix4_T<T>& lhs, const T & rhs) noexcept
    {
        Matrix4_T<T> temp(lhs);
        return temp /= rhs;
    }

    friend bool operator!=(const Matrix4_T<T>& lhs, const Matrix4_T<T>& rhs) noexcept
    {
        return !(lhs == rhs);
    }

    // print
    template <typename U>
    friend std::ostream& operator<<(std::ostream& os, const Matrix4_T<U>& rhs)
    {
        return os << rhs.m_;
    }
private:
	Vector_T<Vector_T<T, col_num>, row_num> m_;
};

using float4x4 = Matrix4_T<float>;
}