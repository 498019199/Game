// @zhangbei 2017��6��16�� 
//�Ѵ�3D��Ϸ������ͼ������Ϸ�����еĴ�����ش���ĳ�C++ģ�� ��Ҫ����verctor,matrix,quatern

#ifndef _STX_MATH_H_
#define _STX_MATH_H_
#pragma 
#include "MathDefine.h"
#include <limits>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <iterator>
namespace MathLib
{
	// tool math 
	///////////////////////////////////////////////////////////////////////////////
	const float PI = 3.141592f;
	const float PI2 = 6.283185f;
	const float PIdiv2 = 1.570796f;

	float const DEG90 = 1.570796f;			// 90 ��
	float const DEG270 = -1.570796f;		// 270 ��
	float const DEG45 = 0.7853981f;			// 45 ��
	float const DEG5 = 0.0872664f;			// 5 ��
	float const DEG10 = 0.1745329f;			// 10 ��
	float const DEG20 = 0.3490658f;			// 20 ��
	float const DEG30 = 0.5235987f;			// 30 ��
	float const DEG60 = 1.047197f;			// 60 ��
	float const DEG120 = 2.094395f;			// 120 ��
	float const DEG40 = 0.6981317f;			// 40 ��
	float const DEG80 = 1.396263f;			// 80 ��
	float const DEG140 = 2.443460f;			// 140 ��
	float const DEG160 = 2.792526f;			// 160 ��

	float const DEG2RAD = 0.01745329f;			// �ǶȻ���������
	float const RAD2DEG = 57.29577f;			// ���Ȼ��Ƕ�����
												// �ǶȻ�����
	template <typename T>
	inline T deg2rad(const T& x) noexcept
	{
		return static_cast<T>(x * DEG2RAD);
	}
	// ���Ȼ��Ƕ�
	template <typename T>
	inline T rad2deg(const T& x) noexcept
	{
		return static_cast<T>(x * RAD2DEG);
	}

	float Sin(float angle) noexcept;
	float Cos(float angle) noexcept;
	float Tan(float angle) noexcept;
	float Asin(float angle) noexcept;
	float Acos(float angle) noexcept;
	float Atan2(float x, float y) noexcept;
	void SinCos(float angle, float& fs, float& fc) noexcept;
	float Log(float x) noexcept;
	float Log10(float x) noexcept;

	template <typename T>
	inline T Sqrt(T x) noexcept
	{
		return static_cast<T>(std::sqrt(x));
	}

	// �����ֵ
	template <typename T>
	inline T Abs(const T& lhs) noexcept
	{
		return (lhs > T(0) ? lhs : -lhs);
	}

	// ���
	template <typename T> 
	inline bool Equal(const T& lhs, const T& rhs) noexcept
	{
		return lhs == rhs;
	}
	template<>
	inline bool Equal<double>(const double& lhs, const double& rhs) noexcept
	{
		return (Abs<double>(lhs - rhs)
			<= std::numeric_limits<double>::epsilon());
	}
	template<>
	inline bool Equal<float>(const float& lhs, const float& rhs) noexcept
	{
		return (Abs<double>(lhs - rhs)
			<= std::numeric_limits<float>::epsilon());
	}

	//  ƽ�����������㷨
	float RecipSqrt(float number) noexcept;

	// ƽ��
	template<typename T> inline
		T Sqr(const T & data) noexcept
	{
		return data * data;
	}

	// ����
	template<typename T> inline T Cube(const T & data) noexcept
	{
		return Sqr(data) * data;
	}

	// ��������
	template <typename T>
	inline T Round(const T& x) noexcept
	{
		return (x > 0) ? static_cast<T>(static_cast<int>(T(0.5) + x)) :
			-static_cast<T>(static_cast<int>(T(0.5) - x));
	}
	template <typename T>
	inline int RoundToInt(T x) 
	{ 
		return static_cast<int>(Round(x));
	}

	// ȡ��
	template <typename T>
	inline T Trunc(T const & x) noexcept
	{
		return static_cast<T>(static_cast<int>(x));
	}

	// ���� val �� low �� high ֮��
	template <typename T>
	inline const T& Clamp(const T& val, const T& low, const T& high) noexcept;

	// ���Բ�ֵ
	template <typename T>
	T Lerp(const T& lhs, const T& rhs, float s) noexcept;

	// ȡ�����
	template <typename T>
	T Maximize(const T& lhs, const T& rhs) noexcept;

	// ȡ����С
	template <typename T>
	T Minimize(const T& lhs, const T& rhs) noexcept;
//����//////////////////////////////////////////////////////////////////////////////////////////////////////////

	// ���
	template<typename T>
	T Cross(const Vector_T<T, 2> & lhs, const Vector_T<T, 2> & rhs) noexcept;
	template<typename T>
	Vector_T<T, 3> Cross(const Vector_T<T, 3> & lhs, const Vector_T<T, 3> & rhs) noexcept;
	template<typename T>
	Vector_T<T, 4> Cross(const Vector_T<T, 4> & lhs, const Vector_T<T, 4> & rhs) noexcept;

	// ���
	template<typename T>
	typename T::value_type Dot(const T & lhs, const T & rhs) noexcept;

	// ���ȵ�ƽ��
	template<typename T>
	typename T::value_type LengthSq(const T & rhs) noexcept;

	// ��ģ�������ĳ�
	template<typename T>
	typename T::value_type Length(const T & rhs) noexcept;

	// ������׼��
	template<typename T>
	T Normalize(const T & rhs) noexcept;

	// ����
	template<typename T>
	T Distance(const Vector_T<T, 2> & lhs, const Vector_T<T, 2> & rhs) noexcept;
	template<typename T>
	T Distance(const Vector_T<T, 3> & lhs, const Vector_T<T, 3> & rhs) noexcept;

// ��ɫ///////////////////////////////////////////////////////////////////////////////
// ��ɫ����

	// ��ɫ�˷�
	template <typename T>
	Color_T<T> Modulate(const Color_T<T>& lhs, const Color_T<T>& rhs) noexcept;

//// ����//////////////////////////////////////////////////////////////////////////////////////////////////////////
	template<typename T>
	Matrix4_T<T> Mul(const Matrix4_T<T>& rhs, const Matrix4_T<T>& lhs) noexcept;

	// ����ת��
	template<typename T>
	Matrix4_T<T> Transpose(const Matrix4_T<T>& mat) noexcept;
	
	// ����
	template<typename T>
	Matrix4_T<T> MatrixScale(const T x, const T y, const T z) noexcept;

	// ����
	template<typename T>
	Matrix4_T<T> MatrixScale(const Vector_T<T, 3>& n) noexcept;

	// ��תx��
	template<typename T>
	Matrix4_T<T> MatrixRotateX(const T & x) noexcept;

	// ��תy��
	template<typename T>
	Matrix4_T<T> MatrixRotateY(const T & y) noexcept;

	// ��תz��
	template<typename T>
	Matrix4_T<T> MatrixRotateZ(const T & z) noexcept;

	// ��ת
	template<typename T>
	Matrix4_T<T> MatrixRotate(const Vector_T<T, 3>& n, const T rotate) noexcept;

	//����ʽֵ
	template<typename T>
	T Determinant(const Matrix4_T<T>& mat) noexcept;

	// �������
	template<typename T>
	Matrix4_T<T> Inverse(const Matrix4_T<T>& mat) noexcept;

	// ����ƽ��
	template<typename T>
	Matrix4_T<T> MatrixMove(const T x, const T y, const T z) noexcept;
	template<typename T>
	Matrix4_T<T> MatrixMove(const Vector_T<T, 3>& n) noexcept;

	// ��ȡ�۲����
	template<typename T>
	Matrix4_T<T> LookAtLH(const Vector_T<T, 3>& vEye, const Vector_T<T, 3>& vAt, const Vector_T<T, 3>& vUp) noexcept;

	// ��ȡ͸��ͶӰ����
	template<typename T>
	Matrix4_T<T>  PerspectiveFovLH(T fFov, T fNearPlane, T fFarPlane, T fAspect) noexcept;

	// �����˾���
	template<typename T>
	Vector_T<T, 4> MatrixMulVector(const Vector_T<T, 4>& a, const Matrix4_T<T>& b) noexcept;

	// ��Ԫ��ת����
	template <typename T>
	Matrix4_T<T> ToMatrix(const Quaternion_T<T>& quat) noexcept;
//// ��Ԫ��//////////////////////////////////////////////////////////////////////////////////////////////////////////������Ϊ�˽��������ת������
	// ��Ԫ���˷�
	template <typename T>
	Quaternion_T<T> Mul(Quaternion_T<T> const & lhs, Quaternion_T<T> const & rhs) noexcept;

	// ��Ԫ������
	template <typename T>
	Quaternion_T<T> Conjugate(Quaternion_T<T> const & rhs) noexcept;

	// ��Ԫ������
	template <typename T>
	Quaternion_T<T> Inverse(const Quaternion_T<T>& rhs) noexcept;

	// ��Ԫ��ת����
	template <typename T>
	Quaternion_T<T> ToQuaternion(const Matrix4_T<T>& mat) noexcept;

	// ŷ����ת��Ԫ��
	template <typename T>
	Quaternion_T<T>  ToQuaternion(const Vector_T<T, 3>& rhs);

	// ��Ԫ���Ķ���
	template <typename T>
	Quaternion_T<T>  Exp(const Quaternion_T<T> & rhs);
	template <typename T>
	Quaternion_T<T>  In(const Quaternion_T<T> & quat);

	// ��Ԫ����ת
	template <typename T>
	Quaternion_T<T> QuaternionRotate(const Vector_T<T, 3>& n, const T rotate);

	// �������Բ�ֵ
	template <typename T>
	Quaternion_T<T>  Slerp(const Quaternion_T<T> & p1, const Quaternion_T<T> & p2, T ft);
template <typename T>
	Quaternion Squad(const Quaternion_T<T> & q1, const Quaternion_T<T> & a,
		const Quaternion_T<T> & b, const Quaternion_T<T> & c, T ft);

	// ��Ԫ��תŷ����
	template <typename T>
	void ToYawPitchRoll(T& yaw, T& pitch, T& roll, Quaternion_T<T> const & quat) noexcept;
	/****************************��ֲ������*****************************/
	template <typename T>
	void decompose(Vector_T<T, 3>& scale, Quaternion_T<T>& rot, Vector_T<T, 3>& trans, Matrix4_T<T> const & rhs) noexcept;
	template <typename T>
	Vector_T<T, 3> transform_quat(Vector_T<T, 3> const & v, Quaternion_T<T> const & quat) noexcept;
// ƽ��
	// �����

	/**************************************�ཻ���//���==��ר�ŵ���ײ����д�� 2017/10/22 17��13��*/
	//// ����x
	//bool IntersectPointToAABB(const float3& v, const AABBox& aabb);
	//bool IntersectPointToOBB(const float3& v, const OBBox& obb);
	//bool IntersectPointToSphere(const float3& v, const Sphere& sphere);
	//// ������x
	//bool IntersectRayToAABB(const float3& org, const float3& dist, const AABBox& aabb);
	//bool IntersectRayToOBB(const float3& org, const float3& dist, const OBBox& obb);
	//bool IntersectRayToSphere(const float3& org, const float3& dist, const Sphere& sphere);
	//// aabb��x
	//bool IntersectAABBToAABB(const AABBox& aabb1, const AABBox& aabb2);
	//bool IntersectAABBToOBB(const AABBox& aabb, const OBBox& obb);
	//bool IntersectAABBToSphere(const AABBox& aabb, const Sphere& sphere);

	//// obb��x
	//bool IntersectOBBToAABB(const OBBox& rhs, const AABBox& lhs);
	//bool IntersectOBBToOBB(const OBBox& rhs, const OBBox& lhs);
	//bool IntersectOBBToSphere(const OBBox& obb, const Sphere& sphere);
	//bool IntersectSphereToSphere(const Sphere& rhs, const Sphere& lhs);
	/**************************************��ż��Ԫ��*******************************************/

	/**************************************����*******************************************/
	// http://www.cnblogs.com/ThreeThousandBigWorld/archive/2012/07/16/2593892.html
	// http://blog.chinaunix.net/uid-26651460-id-3083223.html
	// http://stackoverflow.com/questions/5255806/how-to-calculate-tangent-and-binormal
	// ����TBN��,������ͼ
	template <typename TangentIterator, typename BinormIterator,
		typename IndexIterator, typename PositionIterator, typename TexCoordIterator, typename NormalIterator>
		inline void
		compute_tangent(TangentIterator targentsBegin, BinormIterator binormsBegin,
			IndexIterator indicesBegin, IndexIterator indicesEnd,
			PositionIterator xyzsBegin, PositionIterator xyzsEnd,
			TexCoordIterator texsBegin, NormalIterator normalsBegin) noexcept
	{
		typedef typename std::iterator_traits<PositionIterator>::value_type position_type;
		typedef typename std::iterator_traits<TexCoordIterator>::value_type texcoord_type;
		typedef typename std::iterator_traits<TangentIterator>::value_type tangent_type;
		typedef typename std::iterator_traits<BinormIterator>::value_type binormal_type;
		typedef typename std::iterator_traits<NormalIterator>::value_type normal_type;
		typedef typename position_type::value_type value_type;

		int const num = static_cast<int>(std::distance(xyzsBegin, xyzsEnd));

		for (int i = 0; i < num; ++i)
		{
			*(targentsBegin + i) = tangent_type::Zero();
			*(binormsBegin + i) = binormal_type::Zero();
		}

		for (IndexIterator iter = indicesBegin; iter != indicesEnd; iter += 3)
		{
			uint32_t const v0Index = *(iter + 0);
			uint32_t const v1Index = *(iter + 1);
			uint32_t const v2Index = *(iter + 2);

			position_type const & v0XYZ(*(xyzsBegin + v0Index));
			position_type const & v1XYZ(*(xyzsBegin + v1Index));
			position_type const & v2XYZ(*(xyzsBegin + v2Index));

			Vector_T<value_type, 3> v1v0 = v1XYZ - v0XYZ;
			Vector_T<value_type, 3> v2v0 = v2XYZ - v0XYZ;

			texcoord_type const & v0Tex(*(texsBegin + v0Index));
			texcoord_type const & v1Tex(*(texsBegin + v1Index));
			texcoord_type const & v2Tex(*(texsBegin + v2Index));

			value_type s1 = v1Tex.x() - v0Tex.x();
			value_type t1 = v1Tex.y() - v0Tex.y();

			value_type s2 = v2Tex.x() - v0Tex.x();
			value_type t2 = v2Tex.y() - v0Tex.y();

			value_type denominator = s1 * t2 - s2 * t1;
			Vector_T<value_type, 3> tangent, binormal;
			if (MathLib::Abs(denominator) < std::numeric_limits<value_type>::epsilon())
			{
				tangent = Vector_T<value_type, 3>(1, 0, 0);
				binormal = Vector_T<value_type, 3>(0, 1, 0);
			}
			else
			{
				tangent = (t2 * v1v0 - t1 * v2v0) / denominator;
				binormal = (s1 * v2v0 - s2 * v1v0) / denominator;
			}

			tangent_type t = Vector_T<value_type, 4>(tangent.x(), tangent.y(), tangent.z(), value_type(1));

			*(targentsBegin + v0Index) += t;
			*(binormsBegin + v0Index) += binormal;

			*(targentsBegin + v1Index) += t;
			*(binormsBegin + v1Index) += binormal;

			*(targentsBegin + v2Index) += t;
			*(binormsBegin + v2Index) += binormal;
		}

		for (int i = 0; i < num; ++i)
		{
			tangent_type t(*(targentsBegin + i));
			Vector_T<value_type, 3> tangent(t.x(), t.y(), t.z());
			binormal_type binormal(*(binormsBegin + i));
			normal_type normal(*(normalsBegin + i));

			// Gram-Schmidt orthogonalize
			tangent = Normalize(tangent - normal * dot(tangent, normal));
			// Calculate handedness
			value_type k = 1;
			if (Dot(Cross(normal, tangent), binormal) < 0)
			{
				k = -1;
			}

			*(targentsBegin + i) = Vector_T<value_type, 4>(tangent.x(), tangent.y(), tangent.z(), k);
			*(binormsBegin + i) = Cross(normal, tangent);
		}
	}

	bool ComputeBarycentricCoords3d(float4& res, const float4& p0, const float4& p1, const float4& p2, const float4& p);
}

#include "Vector.h"
#include "Matrix.h"
#include "Plane.h"
#include "Sphere.h"
#include "Quaternion.h"
#include "Color.h"
#include "AABBox.h"
//#include "OBBox.h"
#endif//_STX_MATH_H_

