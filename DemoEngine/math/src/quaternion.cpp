
#include <math/quaternion.h>
#include <math/math.h>
namespace MathWorker
{

template <typename T>
constexpr Quaternion_T<T>::Quaternion_T(const T * rhs) noexcept
	:quat_(rhs)
{
}

template <typename T>
constexpr Quaternion_T<T>::Quaternion_T(const Vector_T<T, 3>& vec, T s) noexcept
{
	this->quat_[0] = vec[0];
	this->quat_[1] = vec[1];
	this->quat_[2] = vec[2];
	this->quat_[3] = s;
}

template <typename T>
Quaternion_T<T>::Quaternion_T(const Quaternion_T & rhs) noexcept
	:quat_(rhs.quat_)
{
}

template <typename T>
Quaternion_T<T>::Quaternion_T(Quaternion_T&& rhs) noexcept
	:quat_(std::move(rhs.quat_))
{
}

template <typename T>
constexpr Quaternion_T<T>::Quaternion_T(T x, T y, T z, T w) noexcept
{
	this->quat_.x() = x;
	this->quat_.y() = y;
	this->quat_.z() = z;
	this->quat_.w() = w;
}

template <typename T>
const Quaternion_T<T>& Quaternion_T<T>::operator+=(const Quaternion_T & rhs) noexcept
{
	this->quat_ += rhs.quat_;
	return *this;
}

template <typename T>
const Quaternion_T<T>& Quaternion_T<T>::operator-=(const Quaternion_T & rhs) noexcept
{
	this->quat_ -= rhs.quat_;
	return *this;
}

template <typename T>
const Quaternion_T<T>& Quaternion_T<T>::operator*=(const Quaternion_T & rhs) noexcept
{
	*this = Mul(*this, rhs);
	return *this;
}

template <typename T>
const Quaternion_T<T>& Quaternion_T<T>::operator*=(T rhs) noexcept
{
	this->quat_ *= static_cast<T>(rhs);
	return *this;
}

template <typename T>
const Quaternion_T<T>& Quaternion_T<T>::operator/=(T rhs) noexcept
{
	this->quat_ /= static_cast<T>(rhs);
	return *this;
}

template <typename T>
Quaternion_T<T>& Quaternion_T<T>::operator=(const Quaternion_T & rhs) noexcept
{
	if (this != &rhs)
	{
		this->quat_ = rhs.quat_;
	}

	return *this;
}

template <typename T>
Quaternion_T<T> Quaternion_T<T>::operator+(const Quaternion_T & rhs) const noexcept
{
    Quaternion_T<T> tmp(*this);
	return tmp.operator+=(rhs);
}

template <typename T>
Quaternion_T<T> Quaternion_T<T>::operator-(const Quaternion_T & rhs) const noexcept
{
    Quaternion_T<T> tmp(*this);
	return tmp.operator-=(rhs);
}

template <typename T>
Quaternion_T<T> Quaternion_T<T>::operator*(const Quaternion_T & rhs) const noexcept
{
    Quaternion_T<T> tmp(*this);
	return tmp.operator*=(rhs);
}

template <typename T>
Quaternion_T<T> Quaternion_T<T>::operator*(T rhs) const noexcept
{
    Quaternion_T<T> tmp(*this);
	return tmp.operator*=(rhs);
}

template <typename T>
Quaternion_T<T> Quaternion_T<T>::operator/(T rhs) const noexcept
{
    Quaternion_T<T> tmp(*this);
	return tmp.operator/=(rhs);
}



template <typename T>
Quaternion_T<T>& Quaternion_T<T>::operator=(Quaternion_T&& rhs) noexcept
{
	this->quat_ = std::move(rhs.quat_);
	return *this;
}

template <typename T>
Quaternion_T<T> const Quaternion_T<T>::operator+() const noexcept
{
	return *this;
}

template <typename T>
Quaternion_T<T> const Quaternion_T<T>::operator-() const noexcept
{
	return 	Quaternion_T<T>(-quat_.x(), -quat_.y(), -quat_.z(), -quat_.w());
}

template <typename T>
const Vector_T<T, 3> Quaternion_T<T>::GetV() const noexcept
{
	return Vector_T<T, 3>(this->x(), this->y(), this->z());
}

template <typename T>
void Quaternion_T<T>::SetV(Vector_T<T, 3> const & rhs) noexcept
{
	this->quat_[0] = rhs[0];
	this->quat_[1] = rhs[1];
	this->quat_[2] = rhs[2];
}


template <typename T>
bool Quaternion_T<T>::operator==(Quaternion_T<T> const & rhs) const noexcept
{
	return this->quat_ == rhs.quat_;
}

template<typename T>
bool Quaternion_T<T>::operator!=(Quaternion_T<T> const & rhs) const noexcept
{
	return this->quat_ != rhs.quat_;
}

template <typename T>
const Quaternion_T<T>& Quaternion_T<T>::Identity() noexcept
{
	static Quaternion_T<T> const out(0, 0, 0, 1);
	return out;
}

// print
template class Quaternion_T<float>;
}