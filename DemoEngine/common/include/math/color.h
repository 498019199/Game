#pragma once
#include <math/vectorxd.h>

namespace RenderWorker
{

template <typename T>
class Color_T
{
public:
	static constexpr size_t elem_num = 4;

	typedef T value_type;
	typedef typename Vector_T<T, elem_num>::pointer pointer;
	typedef typename Vector_T<T, elem_num>::const_pointer const_pointer;
	typedef typename Vector_T<T, elem_num>::reference reference;
	typedef typename Vector_T<T, elem_num>::const_reference const_reference;
	typedef typename Vector_T<T, elem_num>::iterator iterator;
	typedef typename Vector_T<T, elem_num>::const_iterator const_iterator;
public:
	constexpr Color_T() noexcept
	{}
	explicit constexpr Color_T(T const* rhs) noexcept;
	constexpr Color_T(Color_T const& rhs) noexcept;
	constexpr Color_T(Color_T&& rhs) noexcept;
	constexpr Color_T(T r, T g, T b, T a) noexcept;
	Color_T(uint32_t dw) noexcept;

	// 取颜色
	constexpr iterator begin() noexcept
	{
		return col_.begin();
	}
	constexpr const_iterator begin() const noexcept
	{
		return col_.begin();
	}
	constexpr iterator end() noexcept
	{
		return col_.end();
	}
	constexpr const_iterator end() const noexcept
	{
		return col_.end();
	}
	constexpr reference operator[](size_t index) noexcept
	{
		return col_[index];
	}
	constexpr const_reference operator[](size_t index) const noexcept
	{
		return col_[index];
	}

	constexpr reference r() noexcept
	{
		return col_[0];
	}
	constexpr const_reference r() const noexcept
	{
		return col_[0];
	}
	constexpr reference g() noexcept
	{
		return col_[1];
	}
	constexpr const_reference g() const noexcept
	{
		return col_[1];
	}
	constexpr reference b() noexcept
	{
		return col_[2];
	}
	constexpr const_reference b() const noexcept
	{
		return col_[2];
	}
	constexpr reference a() noexcept
	{
		return col_[3];
	}
	constexpr const_reference a() const noexcept
	{
		return col_[3];
	}

	void RGBA(uint8_t& R, uint8_t& G, uint8_t& B, uint8_t& A) const noexcept;
	uint32_t ARGB() const noexcept;
	uint32_t ABGR() const noexcept;

	Color_T& operator=(Color_T const & rhs) noexcept;
	Color_T& operator=(Color_T&& rhs) noexcept;

	Color_T& operator+=(Color_T<T> const & rhs) noexcept;
	Color_T& operator-=(Color_T<T> const & rhs) noexcept;
	Color_T& operator*=(T rhs) noexcept;
	Color_T& operator*=(Color_T<T> const & rhs) noexcept;
	Color_T& operator/=(T rhs) noexcept;

	bool operator==(Color_T<T> const & rhs) const noexcept;

	friend Color_T<T> operator+(const Color_T<T>& lhs, const Color_T<T>&  rhs) noexcept
	{
		Color_T<T> temp(lhs);
		return temp += rhs;
	}
	friend Color_T<T> operator-(const Color_T<T>&  lhs, const Color_T<T>&  rhs) noexcept
	{
		Color_T<T> temp(lhs);
		return temp -= rhs;
	}

	friend Color_T<T> operator*(const Color_T<T>&  lhs, T rhs) noexcept
	{
		Color_T<T> temp(lhs);
		return temp *= rhs;
	}
	friend Color_T<T> operator*(T lhs, const Color_T<T>&  rhs) noexcept
	{
		Color_T<T> temp(rhs);
		return temp *= lhs;
	}
	friend Color_T<T> operator*(const Color_T<T>&  lhs, const Color_T<T>&  rhs) noexcept
	{
		return lhs * rhs;
	}

	friend Color_T<T> operator/(const Color_T<T>&  lhs, T rhs) noexcept
	{
		Color_T<T> temp(lhs);
		return temp /= rhs;
	}

	friend bool operator!=(const Color_T<T>&  lhs, const Color_T<T>& rhs) noexcept
	{
		return !(lhs.col_ == rhs.col_);	
	}
private:
	Vector_T<T, elem_num> col_;
};

using Color = Color_T<float>;
}