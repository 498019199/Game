#include "Quaternion.h"
#include "Math.h"

template <typename T>
constexpr Quaternion_T<T>::Quaternion_T(const T * rhs) noexcept
	:quat(rhs)
{
}

template <typename T>
constexpr Quaternion_T<T>::Quaternion_T(const Vector_T<T, 3>& vec, T s) noexcept
{
	this->quat[0] = vec[0];
	this->quat[1] = vec[1];
	this->quat[2] = vec[2];
	this->quat[3] = s;
}

template <typename T>
Quaternion_T<T>::Quaternion_T(const Quaternion_T & rhs) noexcept
	:quat(rhs.quat)
{
}

template <typename T>
Quaternion_T<T>::Quaternion_T(Quaternion_T&& rhs) noexcept
	:quat(std::move(rhs.quat))
{
}

template <typename T>
constexpr Quaternion_T<T>::Quaternion_T(T x, T y, T z, T w) noexcept
{
	this->quat.x() = x;
	this->quat.y() = y;
	this->quat.z() = z;
	this->quat.w() = w;
}

template <typename T>
const Quaternion_T<T>& Quaternion_T<T>::operator+=(const Quaternion_T & rhs) noexcept
{
	this->quat += rhs.quat;
	return *this;
}

template <typename T>
const Quaternion_T<T>& Quaternion_T<T>::operator-=(const Quaternion_T & rhs) noexcept
{
	this->quat -= rhs.quat;
	return *this;
}

template <typename T>
const Quaternion_T<T>& Quaternion_T<T>::operator*=(const Quaternion_T & rhs) noexcept
{
	*this = MathLib::Mul(*this, rhs);
	return *this;
}

template <typename T>
const Quaternion_T<T>& Quaternion_T<T>::operator*=(T rhs) noexcept
{
	this->quat *= static_cast<T>(rhs);
	return *this;
}

template <typename T>
const Quaternion_T<T>& Quaternion_T<T>::operator/=(T rhs) noexcept
{
	this->quat /= static_cast<T>(rhs);
	return *this;
}

template <typename T>
Quaternion_T<T>& Quaternion_T<T>::operator=(const Quaternion_T & rhs) noexcept
{
	if (this != &rhs)
	{
		this->quat = rhs.quat;
	}

	return *this;
}

template <typename T>
Quaternion_T<T>& Quaternion_T<T>::operator=(Quaternion_T&& rhs) noexcept
{
	this->quat = std::move(rhs.quat);
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
	return 	Quaternion_T<T>(-quat.x(), -quat.y(), -quat.z(), -quat.w());
}

template <typename T>
const Vector_T<T, 3> Quaternion_T<T>::GetV() const noexcept
{
	return Vector_T<T, 3>(this->x(), this->y(), this->z());
}

template <typename T>
void Quaternion_T<T>::SetV(Vector_T<T, 3> const & rhs) noexcept
{
	this->quat[0] = rhs[0];
	this->quat[1] = rhs[1];
	this->quat[2] = rhs[2];
}


template <typename T>
bool Quaternion_T<T>::operator==(Quaternion_T<T> const & rhs) const noexcept
{
	return this->quat == rhs.quat;
}

template <typename T>
const Quaternion_T<T>& Quaternion_T<T>::Identity() noexcept
{
	static Quaternion_T<T> const out(0, 0, 0, 1);
	return out;
}

// ÊµÀý»¯Ä£°å
template class Quaternion_T<float>;
