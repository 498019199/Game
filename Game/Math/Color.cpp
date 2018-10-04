#include "Color.h"
#include "Math.h"

template <typename T>
Color_T<T>::Color_T(Color_T const & rhs) noexcept
	: col(rhs.col)
{
}

template <typename T>
Color_T<T>::Color_T(Color_T&& rhs) noexcept
	: col(std::move(rhs.col))
{
}

template <typename T>
Color_T<T>::Color_T(uint32_t dw) noexcept
{
	static T const f(1 / T(255));
	this->a() = f * (static_cast<T>(static_cast<uint8_t>(dw >> 24)));
	this->r() = f * (static_cast<T>(static_cast<uint8_t>(dw >> 16)));
	this->g() = f * (static_cast<T>(static_cast<uint8_t>(dw >> 8)));
	this->b() = f * (static_cast<T>(static_cast<uint8_t>(dw >> 0)));
}

// …Ë÷√0-255∑∂Œßƒ⁄£¨
template <typename T>
void Color_T<T>::RGBA(uint8_t& R, uint8_t& G, uint8_t& B, uint8_t& A) const noexcept
{
	R = static_cast<uint8_t>(MathLib::Clamp(this->r(), T(0), T(1)) * 255 + 0.5f);
	G = static_cast<uint8_t>(MathLib::Clamp(this->g(), T(0), T(1)) * 255 + 0.5f);
	B = static_cast<uint8_t>(MathLib::Clamp(this->b(), T(0), T(1)) * 255 + 0.5f);
	A = static_cast<uint8_t>(MathLib::Clamp(this->a(), T(0), T(1)) * 255 + 0.5f);
}

template <typename T>
uint32_t Color_T<T>::ARGB() const noexcept
{
	uint8_t r, g, b, a;
	this->RGBA(r, g, b, a);
	return (a << 24) | (r << 16) | (g << 8) | (b << 0);
}

template <typename T>
uint32_t Color_T<T>::ABGR() const noexcept
{
	uint8_t r, g, b, a;
	this->RGBA(r, g, b, a);
	return (a << 24) | (b << 16) | (g << 8) | (r << 0);
}


template <typename T>
const Color_T<T> Color_T<T>::Zero() noexcept
{
	static const Color_T<T>  zero(0,0,0,0);
	return zero;
}

template <typename T>
Color_T<T>& Color_T<T>::operator+=(Color_T<T> const & rhs) noexcept
{
	col += rhs.col;
	return *this;
}

template <typename T>
Color_T<T>& Color_T<T>::operator-=(Color_T<T> const & rhs) noexcept
{
	col -= rhs.col;
	return *this;
}

template <typename T>
Color_T<T>& Color_T<T>::operator*=(T rhs) noexcept
{
	col *= rhs;
	return *this;
}

template <typename T>
Color_T<T>& Color_T<T>::operator*=(Color_T<T> const & rhs) noexcept
{
	*this = MathLib::Modulate(*this, rhs);
	return *this;
}

template <typename T>
Color_T<T>& Color_T<T>::operator/=(T rhs) noexcept
{
	col /= rhs;
	return *this;
}

template <typename T>
Color_T<T>& Color_T<T>::operator=(Color_T<T> const & rhs) noexcept
{
	if (this != &rhs)
	{
		col = rhs.col;
	}
	return *this;
}

template <typename T>
Color_T<T>& Color_T<T>::operator=(Color_T<T>&& rhs) noexcept
{
	col = std::move(rhs.col);
	return *this;
}

template <typename T>
Color_T<T> const Color_T<T>::operator+() const noexcept
{
	return *this;
}

template <typename T>
Color_T<T> const Color_T<T>::operator-() const noexcept
{
	return Color_T(-this->r(), -this->g(), -this->b(), -this->a());
}

template <typename T>
bool Color_T<T>::operator==(Color_T<T> const & rhs) const noexcept
{
	return col == rhs.col;
}


template class Color_T<float>;