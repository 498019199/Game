
#include <math/quaternion.h>
#include <math/math.h>
namespace RenderWorker
{

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
	*this = MathWorker::mul(*this, rhs);
	return *this;
}

template <typename T>
const Quaternion_T<T>& Quaternion_T<T>::operator*=(T rhs) noexcept
{
	quat_ *= rhs;
	return *this;
}

template <typename T>
const Quaternion_T<T>& Quaternion_T<T>::operator/=(T rhs) noexcept
{
	quat_ /= rhs;
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

template <typename T>
void Quaternion_T<T>::v(Vector_T<T, 3> const& rhs) noexcept
{
	// set the vector part (x,y,z) of the quaternion
	this->x() = rhs.x();
	this->y() = rhs.y();
	this->z() = rhs.z();
}

// print
template class Quaternion_T<float>;
}