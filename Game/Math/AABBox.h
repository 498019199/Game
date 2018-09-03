#ifndef _STX_MATH_AABB_
#define _STX_MATH_AABB_
// 轴对齐包围盒 2017年7月1日 zhangbei
// 移植 klayGE 
#pragma once
#include "MathDefine.h"
#include "Vector.h"
#include "Bound.h"
template <typename T>
class AABBox_T final : boost::addable2<AABBox_T<T>, Vector_T<T, 3>,
	boost::subtractable2<AABBox_T<T>, Vector_T<T, 3>,
	boost::multipliable2<AABBox_T<T>, T,
	boost::dividable2<AABBox_T<T>, T,
	boost::andable<AABBox_T<T>,
	boost::orable<AABBox_T<T>,
	boost::equality_comparable<AABBox_T<T>>>>>>>>,
	public Bound_T<T>
{
public:
	constexpr AABBox_T() noexcept
	{
	}
	AABBox_T(Vector_T<T, 3> const & vMin, Vector_T<T, 3> const & vMax) noexcept;
	AABBox_T(Vector_T<T, 3>&& vMin, Vector_T<T, 3>&& vMax) noexcept;
	AABBox_T(AABBox_T<T> const & rhs) noexcept;
	AABBox_T(AABBox_T<T>&& rhs) noexcept;

	// 赋值操作符
	AABBox_T<T>& operator+=(Vector_T<T, 3> const & rhs) noexcept;
	AABBox_T<T>& operator-=(Vector_T<T, 3> const & rhs) noexcept;
	AABBox_T<T>& operator*=(T rhs) noexcept;
	AABBox_T<T>& operator/=(T rhs) noexcept;
	AABBox_T<T>& operator&=(AABBox_T<T> const & rhs) noexcept;
	AABBox_T<T>& operator|=(AABBox_T<T> const & rhs) noexcept;

	AABBox_T<T>& operator=(AABBox_T<T> const & rhs) noexcept;
	AABBox_T<T>& operator=(AABBox_T<T>&& rhs) noexcept;

	// 一元操作符
	AABBox_T<T> const operator+() const noexcept;
	AABBox_T<T> const operator-() const noexcept;

	// 属性
	T Width() const noexcept;
	T Height() const noexcept;
	T Depth() const noexcept;
	virtual bool IsEmpty() const noexcept override;

	Vector_T<T, 3> const LeftBottomNear() const noexcept;
	Vector_T<T, 3> const LeftTopNear() const noexcept;
	Vector_T<T, 3> const RightBottomNear() const noexcept;
	Vector_T<T, 3> const RightTopNear() const noexcept;
	Vector_T<T, 3> const LeftBottomFar() const noexcept;
	Vector_T<T, 3> const LeftTopFar() const noexcept;
	Vector_T<T, 3> const RightBottomFar() const noexcept;
	Vector_T<T, 3> const RightTopFar() const noexcept;

	Vector_T<T, 3>& Min() noexcept
	{
		return v3Min;
	}
	constexpr Vector_T<T, 3> const & Min() const noexcept
	{
		return v3Min;
	}
	Vector_T<T, 3>& Max() noexcept
	{
		return v3Max;
	}
	constexpr Vector_T<T, 3> const & Max() const noexcept
	{
		return v3Max;
	}
	Vector_T<T, 3> Center() const noexcept;
	Vector_T<T, 3> HalfSize() const noexcept;

	virtual bool VecInBound(Vector_T<T, 3> const & v) const noexcept override;
	virtual T MaxRadiusSq() const noexcept override;

	//bool Intersect(AABBox_T<T> const & aabb) const noexcept;
	//bool Intersect(OBBox_T<T> const & obb) const noexcept;
	//bool Intersect(Sphere_T<T> const & sphere) const noexcept;
	//bool Intersect(Frustum_T<T> const & frustum) const noexcept;

	Vector_T<T, 3> Corner(size_t index) const noexcept;

	bool operator==(AABBox_T<T> const & rhs) const noexcept;

private:
	Vector_T<T, 3> v3Min, v3Max;
};
#endif//_STX_MATH_AABB_