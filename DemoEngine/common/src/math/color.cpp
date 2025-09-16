#include <math/color.h>
#include <math/math.h>
#include <utility>
#include <algorithm>

namespace RenderWorker
{

template<typename T>
constexpr Color_T<T>::Color_T(T const* rhs) noexcept
    : col_(rhs)
{
	
}

template<typename T>
constexpr Color_T<T>::Color_T(Color_T const& rhs) noexcept : col_(rhs.col_)
{
}

template<typename T>
constexpr Color_T<T>::Color_T(Color_T&& rhs) noexcept : col_(std::move(rhs.col_))
{
}

template<typename T>
constexpr Color_T<T>::Color_T(T r, T g, T b, T a) noexcept
    : col_(std::move(r), std::move(g), std::move(b), std::move(a))
{
}

template<typename T>
Color_T<T>::Color_T(uint32_t dw) noexcept
{
    T constexpr rcp = 1 / T(255);
    this->a() = rcp * (static_cast<T>(static_cast<uint8_t>(dw >> 24)));
    this->r() = rcp * (static_cast<T>(static_cast<uint8_t>(dw >> 16)));
    this->g() = rcp * (static_cast<T>(static_cast<uint8_t>(dw >> 8)));
    this->b() = rcp * (static_cast<T>(static_cast<uint8_t>(dw >> 0)));
}

template<typename T>
void Color_T<T>::RGBA(uint8_t& R, uint8_t& G, uint8_t& B, uint8_t& A) const noexcept
{
    R = static_cast<uint8_t>(MathWorker::clamp(this->r(), T(0), T(1)) * 255 + 0.5f);
    G = static_cast<uint8_t>(MathWorker::clamp(this->g(), T(0), T(1)) * 255 + 0.5f);
    B = static_cast<uint8_t>(MathWorker::clamp(this->b(), T(0), T(1)) * 255 + 0.5f);
    A = static_cast<uint8_t>(MathWorker::clamp(this->a(), T(0), T(1)) * 255 + 0.5f);
}

template<typename T>
uint32_t Color_T<T>::ARGB() const noexcept
{
    uint8_t r, g, b, a;
    this->RGBA(r, g, b, a);
    return (a << 24) | (r << 16) | (g << 8) | (b << 0);
}

template<typename T>
uint32_t Color_T<T>::ABGR() const noexcept
{
    uint8_t r, g, b, a;
    this->RGBA(r, g, b, a);
    return (a << 24) | (b << 16) | (g << 8) | (r << 0);
}

template<typename T>
Color_T<T>& Color_T<T>::operator=(Color_T const & rhs) noexcept
{
    if (this != &rhs)
    {
        col_ = rhs.col_;
    }
    return *this;	
}

template<typename T>
Color_T<T>& Color_T<T>::operator=(Color_T&& rhs) noexcept
{
    col_ = std::move(rhs.col_);
    return *this;	
}

template<typename T>
Color_T<T>& Color_T<T>::operator/=(T rhs) noexcept
{
    col_ /= rhs;
    return *this;	
}

template<typename T>
Color_T<T>& Color_T<T>::operator*=(Color_T<T> const & rhs) noexcept
{
    *this = Color_T<T>(r() * rhs.r(), 
        g() * rhs.g(),
        b() * rhs.b(),
        a() * rhs.a());
    return *this; 		
}

template<typename T>
Color_T<T>& Color_T<T>::operator-=(Color_T<T> const & rhs) noexcept
{
    col_ -= rhs.col_;
    return *this; 	
}

template<typename T>
Color_T<T>& Color_T<T>::operator*=(T rhs) noexcept
{
    col_ *= rhs;
    return *this;  	
}

template<typename T>
Color_T<T>& Color_T<T>::operator+=(Color_T<T> const & rhs) noexcept
{
    col_ += rhs.col_;
    return *this;    
}

template<typename T>
bool Color_T<T>::operator==(Color_T<T> const & rhs) const noexcept
{
	return col_ == rhs.col_;
}

template class Color_T<float>;
}