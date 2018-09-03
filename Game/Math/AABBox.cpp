#include "AABBox.h"
#include "Math.h"
#include <boost/assert.hpp>
template <typename T>
AABBox_T<T>::AABBox_T(Vector_T<T, 3> const & vMin, Vector_T<T, 3> const & vMax) noexcept
	: v3Min(vMin), v3Max(vMax)
{
	BOOST_ASSERT(v3Min.x() <= v3Max.x());
	BOOST_ASSERT(v3Min.y() <= v3Max.y());
	BOOST_ASSERT(v3Min.z() <= v3Max.z());
}

template <typename T>
AABBox_T<T>::AABBox_T(Vector_T<T, 3>&& vMin, Vector_T<T, 3>&& vMax) noexcept
	: v3Min(std::move(vMin)), v3Max(std::move(vMax))
{
	BOOST_ASSERT(v3Min.x() <= v3Max.x());
	BOOST_ASSERT(v3Min.y() <= v3Max.y());
	BOOST_ASSERT(v3Min.z() <= v3Max.z());
}

template <typename T>
AABBox_T<T>::AABBox_T(AABBox_T<T> const & rhs) noexcept
	: Bound_T<T>(rhs),
	v3Min(rhs.v3Min), v3Max(rhs.v3Max)
{
}

template <typename T>
AABBox_T<T>::AABBox_T(AABBox_T<T>&& rhs) noexcept
	: Bound_T<T>(rhs),
	v3Min(std::move(rhs.v3Min)), v3Max(std::move(rhs.v3Max))
{
}

template <typename T>
AABBox_T<T>& AABBox_T<T>::operator+=(Vector_T<T, 3> const & rhs) noexcept
{
	v3Min += rhs;
	v3Max += rhs;
	return *this;
}

template <typename T>
AABBox_T<T>& AABBox_T<T>::operator-=(Vector_T<T, 3> const & rhs) noexcept
{
	v3Min -= rhs;
	v3Max -= rhs;
	return *this;
}

template <typename T>
AABBox_T<T>& AABBox_T<T>::operator*=(T rhs) noexcept
{
	this->Min() *= rhs;
	this->Max() *= rhs;
	return *this;
}

template <typename T>
AABBox_T<T>& AABBox_T<T>::operator/=(T rhs) noexcept
{
	return this->operator*=(1.0f / rhs);
}

template <typename T>
AABBox_T<T>& AABBox_T<T>::operator&=(AABBox_T<T> const & rhs) noexcept
{
	v3Min = MathLib::Maximize(this->Min(), rhs.Min());
	v3Max = MathLib::Minimize(this->Max(), rhs.Max());
	return *this;
}

template <typename T>
AABBox_T<T>& AABBox_T<T>::operator|=(AABBox_T<T> const & rhs) noexcept
{
	v3Min = MathLib::Minimize(this->Min(), rhs.Min());
	v3Max = MathLib::Maximize(this->Max(), rhs.Max());
	return *this;
}

template <typename T>
AABBox_T<T>& AABBox_T<T>::operator=(AABBox_T<T> const & rhs) noexcept
{
	if (this != &rhs)
	{
		this->Min() = rhs.Min();
		this->Max() = rhs.Max();
	}
	return *this;
}

template <typename T>
AABBox_T<T>& AABBox_T<T>::operator=(AABBox_T<T>&& rhs) noexcept
{
	v3Min = std::move(rhs.v3Min);
	v3Max = std::move(rhs.v3Max);
	return *this;
}

template <typename T>
AABBox_T<T> const AABBox_T<T>::operator+() const noexcept
{
	return *this;
}

template <typename T>
AABBox_T<T> const AABBox_T<T>::operator-() const noexcept
{
	return AABBox_T<T>(-this->Max(), -this->Min());
}

template <typename T>
T AABBox_T<T>::Width() const noexcept
{
	return this->Max().x() - this->Min().x();
}

template <typename T>
T AABBox_T<T>::Height() const noexcept
{
	return this->Max().y() - this->Min().y();
}

template <typename T>
T AABBox_T<T>::Depth() const noexcept
{
	return this->Max().z() - this->Min().z();
}

template <typename T>
bool AABBox_T<T>::IsEmpty() const noexcept
{
	return this->Min() == this->Max();
}

template <typename T>
Vector_T<T, 3> const AABBox_T<T>::LeftBottomNear() const noexcept
{
	return this->Min();
}

template <typename T>
Vector_T<T, 3> const AABBox_T<T>::LeftTopNear() const noexcept
{
	return Vector_T<T, 3>(this->Min().x(), this->Max().y(), this->Min().z());
}

template <typename T>
Vector_T<T, 3> const AABBox_T<T>::RightBottomNear() const noexcept
{
	return Vector_T<T, 3>(this->Max().x(), this->Min().y(), this->Min().z());
}

template <typename T>
Vector_T<T, 3> const AABBox_T<T>::RightTopNear() const noexcept
{
	return Vector_T<T, 3>(this->Max().x(), this->Max().y(), this->Min().z());
}

template <typename T>
Vector_T<T, 3> const AABBox_T<T>::LeftBottomFar() const noexcept
{
	return Vector_T<T, 3>(this->Min().x(), this->Min().y(), this->Max().z());
}

template <typename T>
Vector_T<T, 3> const AABBox_T<T>::LeftTopFar() const noexcept
{
	return Vector_T<T, 3>(this->Min().x(), this->Max().y(), this->Max().z());
}

template <typename T>
Vector_T<T, 3> const AABBox_T<T>::RightBottomFar() const noexcept
{
	return Vector_T<T, 3>(this->Max().x(), this->Min().y(), this->Max().z());
}

template <typename T>
Vector_T<T, 3> const AABBox_T<T>::RightTopFar() const noexcept
{
	return this->Max();
}

template <typename T>
Vector_T<T, 3> AABBox_T<T>::Center() const noexcept
{
	return (v3Min + v3Max) / 2.0f;
}

template <typename T>
Vector_T<T, 3> AABBox_T<T>::HalfSize() const noexcept
{
	return (v3Max - v3Min) / 2.0f;
}

template <typename T>
bool AABBox_T<T>::VecInBound(Vector_T<T, 3> const & v) const noexcept
{
	//return MathLib::intersect_point_aabb(v, *this);
	return false;
}

template <typename T>
T AABBox_T<T>::MaxRadiusSq() const noexcept
{
	return std::max<T>(MathLib::LengthSq(this->Max()), MathLib::LengthSq(this->Min()));
}

//template <typename T>
//bool AABBox_T<T>::Intersect(AABBox_T<T> const & aabb) const noexcept
//{
//	return MathLib::intersect_aabb_aabb(*this, aabb);
//}
//
//template <typename T>
//bool AABBox_T<T>::Intersect(OBBox_T<T> const & obb) const noexcept
//{
//	return MathLib::intersect_aabb_obb(*this, obb);
//}
//
//template <typename T>
//bool AABBox_T<T>::Intersect(Sphere_T<T> const & sphere) const noexcept
//{
//	return MathLib::intersect_aabb_sphere(*this, sphere);
//}
//
//template <typename T>
//bool AABBox_T<T>::Intersect(Frustum_T<T> const & frustum) const noexcept
//{
//	return MathLib::intersect_aabb_frustum(*this, frustum) != BO_No;
//}

template <typename T>
Vector_T<T, 3> AABBox_T<T>::Corner(size_t index) const noexcept
{
	BOOST_ASSERT(index < 8);

	return Vector_T<T, 3>((index & 1UL) ? this->Max().x() : this->Min().x(),
		(index & 2UL) ? this->Max().y() : this->Min().y(),
		(index & 4UL) ? this->Max().z() : this->Min().z());
}

template <typename T>
bool AABBox_T<T>::operator==(AABBox_T<T> const & rhs) const noexcept
{
	return (this->Min() == rhs.Min()) && (this->Max() == rhs.Max());
}


template class AABBox_T<float>;