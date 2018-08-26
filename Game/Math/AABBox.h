#ifndef _STX_MATH_AABB_
#define _STX_MATH_AABB_
#pragma once
#include "MathDefine.h"
#include "Vector.h"

// ������Χ��
template<typename T>
class AABBox
{
public:
	AABBox()
	{}

	AABBox(const float3& _Pmin, const float3& _Pmax);

	AABBox(const AABBox& that);

	AABBox(AABBox&& that);

	//���������
	AABBox& operator+(const float3& that);

	AABBox operator-(const float3& that);

	AABBox& operator*(T that);

	AABBox& operator/(T  that);

	// ��ֵ������
	AABBox& operator+=(const float3& that);

	AABBox& operator-=(const float3& that);

	AABBox& operator*=(T that);

	AABBox& operator/=(T  that);

	AABBox& operator=(const AABBox& that);

	AABBox& operator=(AABBox&& that);

	// һԪ������
	const AABBox& operator+() const;

	const AABBox operator-() const;

	// ����
	float3 GetMin() const
	{
		return this->vMin;
	}

	float3& GetMin()
	{
		return this->vMin;
	}

	float3 GetMax() const
	{
		return this->vMax;
	}

	float3& GetMax()
	{
		return this->vMax;
	}

	T GetWidth() const;

	T GetHeight() const;

	T GetDepth() const;

	float3 GetCenter() const;

	// �ཻ����
	bool Intersect(const Sphere& sphere);

	bool Intersect(const AABBox& aabb);

	bool Intersect(const OBBox_T& obb);

	bool operator==(const AABBox& that);
private:
	float3 vMin;
	float3 vMax;
};
#endif//_STX_MATH_AABB_