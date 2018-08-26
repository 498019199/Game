#ifndef _STX_MATH_OBB_
#define _STX_MATH_OBB_
#pragma once
#include "MathDefine.h"
#include "Vector.h"

//有向包围盒
template<typename T>
class OBBox_T
{
	OBBox_T()
	{}

	OBBox_T(Quaternion& quat, const FmV3& _Pmin, const FmV3& _Pmax);

	OBBox_T(const OBBox_T& that);

	OBBox_T(OBBox_T&& that);

	//运算操作符
	OBBox_T& operator+(const FmV3& that);

	OBBox_T& operator-(const FmV3& that);

	OBBox_T& operator*(len_t that);

	OBBox_T& operator/(len_t  that);

	// 赋值操作符
	OBBox_T& operator+=(const FmV3& that);

	OBBox_T& operator-=(const FmV3& that);

	OBBox_T& operator*=(len_t that);

	OBBox_T& operator/=(len_t  that);

	OBBox_T& operator=(const OBBox_T& that);

	OBBox_T& operator=(OBBox_T&& that);

	// 一元操作符
	const OBBox_T& operator+() const;

	const OBBox_T& operator-() const;

	// 属性
	FmV3 GetCenter() const
	{
		return this->center;
	}

	FmV3& GetCenter()
	{
		return this->center;
	}

	FmV3 GetExtent() const
	{
		return this->extent;
	}

	FmV3& GetExtent()
	{
		return this->extent;
	}

	Quaternion GetRotation() const
	{
		return this->rotation;
	}

	Quaternion& GetRotation()
	{
		return this->rotation;
	}

	// 相交测试
	bool Intersect(const Sphere& sphere);

	bool Intersect(const AABBox& aabb);

	bool Intersect(const OBBox_T& obb);

	bool operator==(const OBBox_T& that);
private:
	Quaternion rotation;
	FmV3 center;
	FmV3 extent;
};
#endif//_STX_MATH_OBB_