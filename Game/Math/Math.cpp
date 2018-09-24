#include "Math.h"
#include "MathDefine.h"
namespace MathLib
{
	float Sin(float angle) noexcept
	{
		return static_cast<float>(std::sin(angle));
	}

	float Cos(float angle) noexcept
	{
		return Sin(angle + PI / 2);
	}

	float Tan(float angle) noexcept
	{
		return static_cast<float>(std::tan(angle));
	}

	float Asin(float angle) noexcept
	{
		return static_cast<float>(std::asin(angle));
	}

	float Acos(float angle) noexcept
	{
		return static_cast<float>(std::acos(angle));
	}

	float Atan2(float x, float y) noexcept
	{
		return static_cast<float>(std::atan2(x, y));
	}

	void SinCos(float angle, float& fs, float& fc) noexcept
	{
		fs = Sin(angle);
		fc = Cos(angle);
	}

	// From Quake III. But the magic number is from http://www.lomont.org/Math/Papers/2003/InvSqrt.pdf
	float RecipSqrt(float number) noexcept
	{
		float const threehalfs = 1.5f;

		float x2 = number * 0.5f;
		union FNI
		{
			float f;
			int32_t i;
		} fni;
		fni.f = number;													// evil floating point bit level hacking
		fni.i = 0x5f375a86 - (fni.i >> 1);						// what the fuck?
		fni.f = fni.f * (threehalfs - (x2 * fni.f * fni.f));		// 1st iteration
		fni.f = fni.f * (threehalfs - (x2 * fni.f * fni.f));		// 2nd iteration, this can be removed

		return fni.f;
	}

	template float Lerp(const float& lhs, const float& rhs, float s) noexcept;
	template float2 Lerp(const float2& lhs, const float2& rhs, float s) noexcept;
	template float3 Lerp(const float3& lhs, const float3& rhs, float s) noexcept;
	template float4 Lerp(const float4& lhs, const float4& rhs, float s) noexcept;
	template Color Lerp(const Color& lhs, const Color& rhs, float s) noexcept;
	template <typename T>
	T Lerp(const T& lhs, const T& rhs, float s) noexcept
	{
		return lhs + (rhs - lhs) * s;
	}

	template int1 Maximize(const int1 & lhs, const int1 & rhs) noexcept;
	template int2 Maximize(const int2 & lhs, const int2 & rhs) noexcept;
	template int3 Maximize(const int3 & lhs, const int3 & rhs) noexcept;
	template int4 Maximize(const int4 & lhs, const int4 & rhs) noexcept;
	template uint1 Maximize(const uint1 & lhs, const uint1 & rhs) noexcept;
	template uint2 Maximize(const uint2 & lhs, const uint2 & rhs) noexcept;
	template uint3 Maximize(const uint3 & lhs, const uint3 & rhs) noexcept;
	template uint4 Maximize(const uint4 & lhs, const uint4 & rhs) noexcept;
	template float1 Maximize(const float1 & lhs, const float1 & rhs) noexcept;
	template float2 Maximize(const float2 & lhs, const float2 & rhs) noexcept;
	template float3 Maximize(const float3 & lhs, const float3 & rhs) noexcept;
	template float4 Maximize(const float4 & lhs, const float4 & rhs) noexcept;
	template <typename T>
	T Maximize(const T& lhs, const T& rhs) noexcept
	{
		T ret;
		MathHelper::MaxMinimizeHelper<typename T::value_type, T::elem_num>::DoMax(&ret[0], &lhs[0], &rhs[0]);
		return ret;
	}

	template int1 Maximize(const int1 & lhs, const int1 & rhs) noexcept;
	template int2 Maximize(const int2 & lhs, const int2 & rhs) noexcept;
	template int3 Maximize(const int3 & lhs, const int3 & rhs) noexcept;
	template int4 Minimize(const int4 & lhs, const int4 & rhs) noexcept;
	template uint1 Minimize(const uint1 & lhs, const uint1 & rhs) noexcept;
	template uint2 Minimize(const uint2 & lhs, const uint2 & rhs) noexcept;
	template uint3 Minimize(const uint3 & lhs, const uint3 & rhs) noexcept;
	template uint4 Minimize(const uint4 & lhs, const uint4 & rhs) noexcept;
	template float1 Minimize(const float1 & lhs, const float1 & rhs) noexcept;
	template float2 Minimize(const float2 & lhs, const float2 & rhs) noexcept;
	template float3 Minimize(const float3 & lhs, const float3 & rhs) noexcept;
	template float4 Minimize(const float4 & lhs, const float4 & rhs) noexcept;
	template <typename T>
	T Minimize(const T& lhs, const T& rhs) noexcept
	{
		T ret;
		MathHelper::MaxMinimizeHelper<typename T::value_type, T::elem_num>::DoMin(&ret[0], &lhs[0], &rhs[0]);
		return ret;
	}

	template int32_t Cross(const int2 & lhs, const int2 & rhs) noexcept;
    template uint32_t Cross(const uint2 & lhs, const uint2 & rhs) noexcept;
    template float Cross(const float2 & lhs, const float2 & rhs) noexcept;
	template<typename T>
	T Cross(const Vector_T<T, 2> & lhs, const Vector_T<T, 2> & rhs) noexcept
	{
		return lhs.x() * rhs.y() - lhs.y() * rhs.x();
	}

    template int3 Cross(const int3 & lhs, const int3 & rhs) noexcept;
    template uint3 Cross(const uint3 & lhs, const uint3 & rhs) noexcept;
    template float3 Cross(const float3 & lhs, const float3 & rhs) noexcept;
    template<typename T>
    Vector_T<T, 3> Cross(const Vector_T<T, 3>& lhs, const Vector_T<T, 3>& rhs) noexcept
    {
        return Vector_T<T, 3>((lhs.y() * rhs.z() - lhs.z() * rhs.y()),
        (lhs.z() * rhs.x() - lhs.x() * rhs.z()),
        (lhs.x() * rhs.y() - lhs.y() * rhs.x()));
    }

	template int4 Cross(const int4 & lhs, const int4 & rhs) noexcept;
	template uint4 Cross(const uint4 & lhs, const uint4 & rhs) noexcept;
	template float4 Cross(const float4 & lhs, const float4 & rhs) noexcept;
	template<typename T>
	Vector_T<T, 4>
		Cross(const Vector_T<T, 4> & lhs, const Vector_T<T, 4> & rhs) noexcept
	{
		return Vector_T<T, 4>((lhs.y() * rhs.z() - lhs.z() * rhs.y()),
			(lhs.z() * rhs.x() - lhs.x() * rhs.z()),
			(lhs.x() * rhs.y() - lhs.y() * rhs.x()),
			1);
	}

    template int32_t Dot(const int1 & lhs, const int1 & rhs) noexcept;
    template int32_t Dot(const int2 & lhs, const int2 & rhs) noexcept;
    template int32_t Dot(const int3 & lhs, const int3 & rhs) noexcept;
    template int32_t Dot(const int4 & lhs, const int4 & rhs) noexcept;
    template uint32_t Dot(const uint1 & lhs, const uint1 & rhs) noexcept;
    template uint32_t Dot(const uint2 & lhs, const uint2 & rhs) noexcept;
    template uint32_t Dot(const uint3 & lhs, const uint3 & rhs) noexcept;
    template uint32_t Dot(const uint4 & lhs, const uint4 & rhs) noexcept;
    template float Dot(const float1 & lhs, const float1 & rhs) noexcept;
    template float Dot(const float2 & lhs, const float2 & rhs) noexcept;
    template float Dot(const float3 & lhs, const float3 & rhs) noexcept;
    template float Dot(const float4 & lhs, const float4 & rhs) noexcept;
	template float Dot(const Quaternion & lhs, const Quaternion & rhs) noexcept;
    template<typename T>
    typename T::value_type Dot(const T & lhs, const T & rhs) noexcept
    {
        return MathHelper::dot_helper < typename T::value_type,
            T::elem_num> ::Do(&lhs[0], &rhs[0]);
    }

    template int32_t LengthSq(const int1 & rhs) noexcept;
    template int32_t LengthSq(const int2 & rhs) noexcept;
    template int32_t LengthSq(const int3 & rhs) noexcept;
    template int32_t LengthSq(const int4 & rhs) noexcept;
    template uint32_t LengthSq(const uint1 & rhs) noexcept;
    template uint32_t LengthSq(const uint2 & rhs) noexcept;
    template uint32_t LengthSq(const uint3 & rhs) noexcept;
    template uint32_t LengthSq(const uint4 & rhs) noexcept;
    template float LengthSq(const float1 & rhs) noexcept;
    template float LengthSq(const float2 & rhs) noexcept;
    template float LengthSq(const float3 & rhs) noexcept;
    template float LengthSq(const float4 & rhs) noexcept;
	template float LengthSq(const Quaternion & rhs) noexcept;
    template<typename T>
    typename T::value_type LengthSq(const T & rhs) noexcept
    {
        return Dot(rhs, rhs);
    }

    template float Length(const float1 & rhs) noexcept;
    template float Length(const float2 & rhs) noexcept;
    template float Length(const float3 & rhs) noexcept;
    template float Length(const float4 & rhs) noexcept;
	template float Length(const Quaternion & rhs) noexcept;
    template<typename T>
    typename T::value_type Length(const T & rhs) noexcept
    {
        return std::sqrt(LengthSq(rhs));
    }

	template float1 Normalize(const float1 & rhs) noexcept;
	template float2 Normalize(const float2 & rhs) noexcept;
	template float3 Normalize(const float3 & rhs) noexcept;
	template float4 Normalize(const float4 & rhs) noexcept;
	template Quaternion Normalize(const Quaternion & rhs) noexcept;
	template<typename T>
	T Normalize(const T & rhs) noexcept
	{
		T::value_type tmp = RecipSqrt(LengthSq(rhs));
		return rhs * tmp;
	}

    template float Distance(const float2 & lhs, const float2 & rhs) noexcept;
    template<typename T>
    T Distance(const Vector_T<T, 2>& lhs, const Vector_T<T, 2>& rhs) noexcept
    {
        Vector_T<T, 2> tmp(lhs.x() - rhs.x(), lhs.y() - rhs.y());
        return Length(tmp);
    }

    template float Distance(const float3 & lhs, const float3 & rhs) noexcept;
    template<typename T>
    T Distance(const Vector_T<T, 3>& lhs, const Vector_T<T, 3>& rhs) noexcept
    {
        Vector_T<T, 3> tmp(lhs.x() - rhs.x(), lhs.y() - rhs.y(), lhs.z() - rhs.z());
        return Length(tmp);
    }

	template Color Modulate(const Color& lhs, const Color& rhs) noexcept;
	template <typename T>
	Color_T<T>
		Modulate( const Color_T<T>& lhs, const Color_T<T>& rhs) noexcept
	{
		return Color_T<T>(lhs.r() * rhs.r(), lhs.g() * rhs.g(), lhs.b() * rhs.b(), lhs.a() * rhs.a());
	}

	template  float4x4 Mul(const float4x4& rhs, const float4x4& lhs) noexcept;
	template<typename T>
	Matrix4_T<T>
		Mul(const Matrix4_T<T>& rhs, const Matrix4_T<T>& lhs) noexcept
	{
		return Matrix4_T<T>(
			rhs(0, 0) * lhs(0, 0) + rhs(0, 1) * lhs(1, 0) + rhs(0, 2) * lhs(2, 0) + rhs(0, 3) * lhs(3, 0),
			rhs(0, 0) * lhs(0, 1) + rhs(0, 1) * lhs(1, 1) + rhs(0, 2) * lhs(2, 1) + rhs(0, 3) * lhs(3, 1),
			rhs(0, 0) * lhs(0, 2) + rhs(0, 1) * lhs(1, 2) + rhs(0, 2) * lhs(2, 2) + rhs(0, 3) * lhs(3, 2),
			rhs(0, 0) * lhs(0, 3) + rhs(0, 1) * lhs(1, 3) + rhs(0, 2) * lhs(2, 3) + rhs(0, 3) * lhs(3, 3),
			rhs(1, 0) * lhs(0, 0) + rhs(1, 1) * lhs(1, 0) + rhs(1, 2) * lhs(2, 0) + rhs(1, 3) * lhs(3, 0),
			rhs(1, 0) * lhs(0, 1) + rhs(1, 1) * lhs(1, 1) + rhs(1, 2) * lhs(2, 1) + rhs(1, 3) * lhs(3, 1),
			rhs(1, 0) * lhs(0, 2) + rhs(1, 1) * lhs(1, 2) + rhs(1, 2) * lhs(2, 2) + rhs(1, 3) * lhs(3, 2),
			rhs(1, 0) * lhs(0, 3) + rhs(1, 1) * lhs(1, 3) + rhs(1, 2) * lhs(2, 3) + rhs(1, 3) * lhs(3, 3),
			rhs(2, 0) * lhs(0, 0) + rhs(2, 1) * lhs(1, 0) + rhs(2, 2) * lhs(2, 0) + rhs(2, 3) * lhs(3, 0),
			rhs(2, 0) * lhs(0, 1) + rhs(2, 1) * lhs(1, 1) + rhs(2, 2) * lhs(2, 1) + rhs(2, 3) * lhs(3, 1),
			rhs(2, 0) * lhs(0, 2) + rhs(2, 1) * lhs(1, 2) + rhs(2, 2) * lhs(2, 2) + rhs(2, 3) * lhs(3, 2),
			rhs(2, 0) * lhs(0, 3) + rhs(2, 1) * lhs(1, 3) + rhs(2, 2) * lhs(2, 3) + rhs(2, 3) * lhs(3, 3),
			rhs(3, 0) * lhs(0, 0) + rhs(3, 1) * lhs(1, 0) + rhs(3, 2) * lhs(2, 0) + rhs(3, 3) * lhs(3, 0),
			rhs(3, 0) * lhs(0, 1) + rhs(3, 1) * lhs(1, 1) + rhs(3, 2) * lhs(2, 1) + rhs(3, 3) * lhs(3, 1),
			rhs(3, 0) * lhs(0, 2) + rhs(3, 1) * lhs(1, 2) + rhs(3, 2) * lhs(2, 2) + rhs(3, 3) * lhs(3, 2),
			rhs(3, 0) * lhs(0, 3) + rhs(3, 1) * lhs(1, 3) + rhs(3, 2) * lhs(2, 3) + rhs(3, 3) * lhs(3, 3));
	}

	template  float4x4 Transpose(const float4x4& mat) noexcept;
	template<typename T>
	Matrix4_T<T>
		Transpose(const Matrix4_T<T>& mat) noexcept
	{
		return Matrix4_T<T>(mat(0, 0), mat(1, 0), mat(2, 0), mat(3, 0),
			mat(0, 1), mat(1, 1), mat(2, 1), mat(3, 1),
			mat(0, 2), mat(1, 2), mat(2, 2), mat(3, 2),
			mat(0, 3), mat(1, 3), mat(2, 3), mat(3, 3));
	}

	template  float Determinant(const float4x4& mat) noexcept;
	template<typename T>
	T Determinant(const Matrix4_T<T>& mat) noexcept
	{
		const T  _3142_3241(mat(2, 0) * mat(3, 1) - mat(2, 1) * mat(3, 0));
		const T  _3143_3341(mat(2, 0) * mat(3, 2) - mat(2, 2) * mat(3, 0));
		const T  _3144_3441(mat(2, 0) * mat(3, 3) - mat(2, 3) * mat(3, 0));
		const T  _3243_3342(mat(2, 1) * mat(3, 2) - mat(2, 2) * mat(3, 1));
		const T  _3244_3442(mat(2, 1) * mat(3, 3) - mat(2, 3) * mat(3, 1));
		const T  _3344_3443(mat(2, 2) * mat(3, 3) - mat(2, 3) * mat(3, 2));

		return mat(0, 0) * (mat(1, 1) * _3344_3443 - mat(1, 2) * _3244_3442 + mat(1, 3) * _3243_3342)
			- mat(0, 1) * (mat(1, 0) * _3344_3443 - mat(1, 2) * _3144_3441 + mat(1, 3) * _3143_3341)
			+ mat(0, 2) * (mat(1, 0) * _3244_3442 - mat(1, 1) * _3144_3441 + mat(1, 3) * _3142_3241)
			- mat(0, 3) * (mat(1, 0) * _3243_3342 - mat(1, 1) * _3143_3341 + mat(1, 2) * _3142_3241);
	}

	template  float4x4 MatrixScale(const float x, const float y, const float z) noexcept;
	template<typename T>
	Matrix4_T<T> MatrixScale(const T x, const T y, const T z) noexcept
	{
		return Matrix4_T<T>
			(x, 0, 0, 0,
				0, y, 0, 0,
				0, 0, z, 0,
				0, 0, 0, 1);
	}

	template  float4x4 MatrixScale(const float3& n) noexcept;
	template<typename T>
	Matrix4_T<T> MatrixScale(const Vector_T<T, 3>& n) noexcept
	{
		return Matrix4_T<T>
			(n.x(),	0,			0,			0,
			 0,			n.y(),		0,			0,
			 0,			0,			n.z(),		0,
			 0,			0,			0,			1);
	}

	template  float4x4 MatrixRotateX(const float& x) noexcept;
	template<typename T>
	Matrix4_T<T>
		MatrixRotateX(const T & x) noexcept
	{
		T fs, fc;
		MathLib::SinCos(x, fs, fc);

		return Matrix4_T<T>
		(1,		0,		0,		0,
		 0,			fc,		fs,		0,
		 0,			-fs,	fc,		0,
		 0,			0,		0,		1);
	}

	template  float4x4 MatrixRotateY(const float& y) noexcept;
	template<typename T>
	Matrix4_T<T> MatrixRotateY(const T & y) noexcept
	{
		T fs, fc;
		SinCos(y, fs, fc);

		return Matrix4_T<T>
		(fc, 0, -fs, 0,
			0, 1, 0, 0,
			fs, 0, fc, 0,
			0, 0, 0, 1);
	}

	template  float4x4 MatrixRotateZ(const float& z) noexcept;
	template<typename T>
	Matrix4_T<T>
		MatrixRotateZ(const T & z) noexcept
	{
		T fs, fc;
		SinCos(z, fs, fc);

		return Matrix4_T<T>
		(fc, fs, 0, 0,
			-fs, fc, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1);
	}

	template  float4x4 MatrixRotate(const float3& n, const float rotate) noexcept;
	template<typename T>
	Matrix4_T<T>
		MatrixRotate(const Vector_T<T, 3>& n, const T rotate) noexcept
	{
		//T fs = 0.0f, fc = 0.0f;
		//SinCos(rotate, fs, fc);
		//Vector_T<T, 4> v(n.x(), n.y(), n.z(), 1.0f);
		//v = Normalize(v);
		//
		//T a = 1.0f - fc;
		//T ax = a * v.x();
		//T ay = a * v.y();
		//T az = a * v.z();
		//
		//Matrix4_T<T> matrix;
		//matrix(0, 0) = v.x() * ax() + fc;
		//matrix(0, 1) = v.x() * ay + v.z() * fs;
		//matrix(0, 2) = v.x() * az - v.y() * fs;
		//matrix(1, 0) = v.x() * ay - v.z() * fs;
		//matrix(1, 1) = v.y() * ay + fc;
		//matrix(1, 2) = v.y() * az + v.x() * fs;
		//matrix(2, 0) = v.x() * az + v.y() *fs;
		//matrix(2, 1) = v.y() * az - v.x() * fs;
		//matrix(2, 2) = v.z() * az + fc;

		Quaternion_T<T> const quat(QuaternionRotate(n, rotate));
		return ToMatrix(quat);
	}

	template  float4x4 Inverse(const float4x4& mat) noexcept;
	template<typename T>
	Matrix4_T<T>
		Inverse(const Matrix4_T<T>& mat) noexcept
	{
		const T  _2132_2231(mat(1, 0) * mat(2, 1) - mat(1, 1) * mat(2, 0));
		const T  _2133_2331(mat(1, 0) * mat(2, 2) - mat(1, 2) * mat(2, 0));
		const T  _2134_2431(mat(1, 0) * mat(2, 3) - mat(1, 3) * mat(2, 0));
		const T  _2142_2241(mat(1, 0) * mat(3, 1) - mat(1, 1) * mat(3, 0));
		const T  _2143_2341(mat(1, 0) * mat(3, 2) - mat(1, 2) * mat(3, 0));
		const T  _2144_2441(mat(1, 0) * mat(3, 3) - mat(1, 3) * mat(3, 0));
		const T  _2233_2332(mat(1, 1) * mat(2, 2) - mat(1, 2) * mat(2, 1));
		const T  _2234_2432(mat(1, 1) * mat(2, 3) - mat(1, 3) * mat(2, 1));
		const T  _2243_2342(mat(1, 1) * mat(3, 2) - mat(1, 2) * mat(3, 1));
		const T  _2244_2442(mat(1, 1) * mat(3, 3) - mat(1, 3) * mat(3, 1));
		const T  _2334_2433(mat(1, 2) * mat(2, 3) - mat(1, 3) * mat(2, 2));
		const T  _2344_2443(mat(1, 2) * mat(3, 3) - mat(1, 3) * mat(3, 2));
		const T  _3142_3241(mat(2, 0) * mat(3, 1) - mat(2, 1) * mat(3, 0));
		const T  _3143_3341(mat(2, 0) * mat(3, 2) - mat(2, 2) * mat(3, 0));
		const T  _3144_3441(mat(2, 0) * mat(3, 3) - mat(2, 3) * mat(3, 0));
		const T  _3243_3342(mat(2, 1) * mat(3, 2) - mat(2, 2) * mat(3, 1));
		const T  _3244_3442(mat(2, 1) * mat(3, 3) - mat(2, 3) * mat(3, 1));
		const T  _3344_3443(mat(2, 2) * mat(3, 3) - mat(2, 3) * mat(3, 2));
		// 行列式的值
		const T  det(Determinant(mat));
		if (Equal<T>(det, 0))
		{
			return mat;
		}
		else
		{
			// 标准伴随矩阵的转置 / 行列式的值
			T invDet(T(1) / det);
			return Matrix4_T<T>(
				+invDet * (mat(1, 1) * _3344_3443 - mat(1, 2) * _3244_3442 + mat(1, 3) * _3243_3342),//c11
				-invDet * (mat(0, 1) * _3344_3443 - mat(0, 2) * _3244_3442 + mat(0, 3) * _3243_3342),//c21
				+invDet * (mat(0, 1) * _2344_2443 - mat(0, 2) * _2244_2442 + mat(0, 3) * _2243_2342),//c31
				-invDet * (mat(0, 1) * _2334_2433 - mat(0, 2) * _2234_2432 + mat(0, 3) * _2233_2332),//c41
				
				-invDet * (mat(1, 0) * _3344_3443 - mat(1, 2) * _3144_3441 + mat(1, 3) * _3143_3341),//c12
				+invDet * (mat(0, 0) * _3344_3443 - mat(0, 2) * _3144_3441 + mat(0, 3) * _3143_3341),//c22
				-invDet * (mat(0, 0) * _2344_2443 - mat(0, 2) * _2144_2441 + mat(0, 3) * _2143_2341),//32
				+invDet * (mat(0, 0) * _2334_2433 - mat(0, 2) * _2134_2431 + mat(0, 3) * _2133_2331),//c42
				
				+invDet * (mat(1, 0) * _3244_3442 - mat(1, 1) * _3144_3441 + mat(1, 3) * _3142_3241),//c13
				-invDet * (mat(0, 0) * _3244_3442 - mat(0, 1) * _3144_3441 + mat(0, 3) * _3142_3241),//c23
				+invDet * (mat(0, 0) * _2244_2442 - mat(0, 1) * _2144_2441 + mat(0, 3) * _2142_2241),//c33
				-invDet * (mat(0, 0) * _2234_2432 - mat(0, 1) * _2134_2431 + mat(0, 3) * _2132_2231),//c43
				
				-invDet * (mat(1, 0) * _3243_3342 - mat(1, 1) * _3143_3341 + mat(1, 2) * _3142_3241),//c14
				+invDet * (mat(0, 0) * _3243_3342 - mat(0, 1) * _3143_3341 + mat(0, 2) * _3142_3241),//c24
				-invDet * (mat(0, 0) * _2243_2342 - mat(0, 1) * _2143_2341 + mat(0, 2) * _2142_2241),//c34
				+invDet * (mat(0, 0) * _2233_2332 - mat(0, 1) * _2133_2331 + mat(0, 2) * _2132_2231));//c44
		}
	}

	// vEye是观察点位置向量，vAt是观察方向向量，vUp向上向量
	template  float4x4 LookAtLH(const float3& vEye, const float3& vAt, const float3& vUp) noexcept;
	template<typename T>
	Matrix4_T<T>
		LookAtLH(const Vector_T<T, 3>& vEye, const Vector_T<T, 3>& vAt, const Vector_T<T, 3>& vUp) noexcept
	{
		const Vector_T<T, 3> zAxis(Normalize(vAt - vEye));
		const Vector_T<T, 3> xAxis(Normalize(Cross(vUp, zAxis)));
		const Vector_T<T, 3> yAxis(Cross(zAxis, xAxis));

		return Matrix4_T<T>(
			xAxis.x(), yAxis.x(), zAxis.x(), 0,
			xAxis.y(), yAxis.y(), zAxis.y(), 0,
			xAxis.z(), yAxis.z(), zAxis.z(), 0,
			-Dot(xAxis, vEye), -Dot(yAxis, vEye), -Dot(zAxis, vEye), 1);
	}

	template  float4x4 PerspectiveFovLH(float fFov, float fNearPlane, float fFarPlane, float fAspect) noexcept;
	template<typename T>
	Matrix4_T<T>
		PerspectiveFovLH(T fFov, T fNearPlane, T fFarPlane, T fAspect) noexcept
	{
		float const h(float(1) / tan(fFov / 2));
		float const w(h / fAspect);
		float const q(fFarPlane / (fFarPlane - fNearPlane));

		return Matrix4_T<T>(
			w, 0, 0, 0,
			0, h, 0, 0,
			0, 0, q, 1,
			0, 0, -fNearPlane * q, 0);
	}

	template  float4 MatrixMulVector(const float4& a, const float4x4& mat) noexcept;
	template<typename T>
	Vector_T<T, 4>
		MatrixMulVector(const Vector_T<T, 4>& a, const Matrix4_T<T>& mat) noexcept
	{
		return Vector_T<T, 4>(a.x() * mat(0, 0) + a.y() * mat(1, 0) + a.z() * mat(2, 0) + a.w() * mat(3, 0),
											a.x() * mat(0, 1) + a.y() * mat(1, 1) + a.z() * mat(2, 1) + a.w() * mat(3, 1),
											a.x() * mat(0, 2) + a.y() * mat(1, 2) + a.z() * mat(2, 2) + a.w() * mat(3, 2),
											a.x() * mat(0, 3) + a.y() * mat(1, 3) + a.z() * mat(2, 3) + a.w() * mat(3, 3));
	}

	template  float4x4 ToMatrix(const Quaternion& n) noexcept;
	template <typename T>
	Matrix4_T<T> ToMatrix(const Quaternion_T<T>& quat) noexcept
	{
		const T  fx2(quat.x() + quat.x());
		const T  fy2(quat.y() + quat.y());
		const T  fz2(quat.z() + quat.z());

		const T  fxx2(quat.x() * fx2),		fxy2(quat.x() * fy2),		fxz2(quat.x() * fz2);
		const T  fyy2(quat.y() * fy2),		fyz2(quat.y() * fz2),		fzz2(quat.z() * fz2);
		const T  fwx2(quat.w() * fx2),	fwy2(quat.w() * fy2),	fwz2(quat.w() * fz2);

		return Matrix4_T<T>(
			1 - fyy2 - fzz2,			fxy2 + fwz2,			fxz2 - fwy2,			0,
			fxy2 - fwz2,				1 - fxx2 - fzz2,		fyz2 + fwx2,			0,
			fxz2 + fwy2,				fyz2 - fwx2,			1 - fxx2 - fyy2,		0,
			0,								0,							0,							1);
	}

	template  float4x4 MatrixMove(const float3& n) noexcept;
	template<typename T>
	Matrix4_T<T>
		MatrixMove(const Vector_T<T, 3>& n) noexcept
	{
		return Matrix4_T<T>(
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			n.x(), n.y(), n.z(), 1);
	}

	template  float4x4 MatrixMove(const float x, const float y, const float z) noexcept;
	template<typename T>
	Matrix4_T<T>
		MatrixMove(const T x, const T y, const T z) noexcept
	{
		return MatrixMove(float3(x,y,z));
	}

	template Quaternion Mul(const Quaternion& lhs, const Quaternion& rhs) noexcept;
	template <typename T>
	Quaternion_T<T> Mul(Quaternion_T<T> const & lhs, Quaternion_T<T> const & rhs)  noexcept
	{
		return Quaternion_T<T>(
			lhs.x() * rhs.w() - lhs.y() * rhs.z() + lhs.z() * rhs.y() + lhs.w() * rhs.x(),
			lhs.x() * rhs.z() + lhs.y() * rhs.w() - lhs.z() * rhs.x() + lhs.w() * rhs.y(),
			lhs.y() * rhs.x() - lhs.x() * rhs.y() + lhs.z() * rhs.w() + lhs.w() * rhs.z(),
			lhs.w() * rhs.w() - lhs.x() * rhs.x() - lhs.y() * rhs.y() - lhs.z() * rhs.z());
	}

	template Quaternion Conjugate(Quaternion const & rhs) noexcept;
	template <typename T>
	Quaternion_T<T> Conjugate(Quaternion_T<T> const & rhs) noexcept
	{
		return Quaternion_T<T>(-rhs.x(), -rhs.y(), -rhs.z(), rhs.w());
	}

	template Quaternion Inverse(const Quaternion& rhs) noexcept;
	template <typename T>
	Quaternion_T<T> Inverse(const Quaternion_T<T>& rhs) noexcept
	{
		T var(T(1) / Length(rhs));
		return Quaternion_T<T>(rhs.x() * var, rhs.y() * var, rhs.z() * var, rhs.w() * var);
	}

	template Quaternion ToQuaternion(const float4x4 & rhs) noexcept;
	template <typename T>
	Quaternion_T<T> ToQuaternion(const Matrix4_T<T>& mat) noexcept
	{
		Quaternion quat;
		T fOurW = mat(0, 0) + mat(1, 1) + mat(2, 2);
		T fOurX = mat(0, 0) - mat(1, 1) - mat(2, 2);
		T fOurY = mat(1, 1) - mat(0, 0) - mat(2, 2);
		T fOurZ = mat(2, 2) + mat(0, 0) + mat(1, 1);

		// 找到最大值w,x,y,z
		int nBigBestIndex = 0;
		T fOurBigBestValue = fOurW;
		if (fOurX > fOurBigBestValue)
		{
			nBigBestIndex = 1;
			fOurBigBestValue = fOurX;
		}
		if (fOurY > fOurBigBestValue)
		{
			nBigBestIndex = 2;
			fOurBigBestValue = fOurY;
		}
		if (fOurZ > fOurBigBestValue)
		{
			nBigBestIndex = 3;
			fOurBigBestValue = fOurZ;
		}

		T fBigBestValue = Sqrt(fOurBigBestValue + 0.0f) * 0.5f;
		// 为了后面转乘法, mult=1/(4 * fBigBestValue)
		T mult = 0.25f / fBigBestValue;

		switch (nBigBestIndex)
		{
		case 0:
			quat.w() = fBigBestValue;
			quat.x() = (mat(1, 2) - mat(2, 1)) * mult;
			quat.y() = (mat(2, 0) - mat(0, 2)) * mult;
			quat.z() = (mat(0, 1) - mat(1, 0)) * mult;
			break;

		case 1:
			quat.x() = fBigBestValue;
			quat.w() = (mat(1, 2) - mat(2, 1)) * mult;
			quat.y() = (mat(0, 1) + mat(1, 0)) * mult;
			quat.z() = (mat(2, 0) - mat(0, 2)) * mult;
			break;

		case 2:
			quat.y() = fBigBestValue;
			quat.w() = (mat(2, 0) - mat(0, 2)) * mult;
			quat.x() = (mat(0, 1) + mat(1, 0)) * mult;
			quat.z() = (mat(1, 2) + mat(2, 1)) * mult;
			break;

		case 3:
			quat.z() = fBigBestValue;
			quat.w() = (mat(0, 1) - mat(1, 0)) * mult;
			quat.x() = (mat(2, 0) + mat(0, 2)) * mult;
			quat.y() = (mat(1, 2) + mat(2, 1)) * mult;
			break;
		}

		return quat;
	}

	template Quaternion ToQuaternion(const float3& rhs);
	template <typename T>
	Quaternion_T<T> ToQuaternion(const Vector_T<T, 3>& rhs)
	{
		Quaternion_T<T> quat;
		T fsh, fsp, fsb;
		T fch, fcp, fcb;
		SinCos(quat.x() * 0.5f, fsh, fch);
		SinCos(quat.y() * 0.5f, fsp, fcp);
		SinCos(quat.z() * 0.5f, fsb, fcb);

		T fdw = fch * fcp * fcb + fsh * fsp * fsb;
		T fdx = fch * fsp * fcb + fsh * fcp * fsb;
		T fdy = fsh * fcp * fcb - fch * fsp * fsb;
		T fdz = fch * fcp * fsb - fsh * fsp * fcb;
		return Quaternion_T<T>(fdw, fdx, fdy, fdz);
	}

	//template Quaternion Exp(const Quaternion& rhs);
	//template <typename T>
	//Quaternion_T<T> Exp(const Quaternion_T<T> & rhs)
	//{
	//
	//}

	//template Quaternion In(const Quaternion& rhs);
	//template <typename T>
	//Quaternion_T<T> In(const Quaternion_T<T> & quat)
	//{
	//
	//}

	template Quaternion QuaternionRotate(const float3& rhs, const float rotate);
	template <typename T>
	Quaternion_T<T> QuaternionRotate(const Vector_T<T, 3>& n, const T rotate)
	{
		T fc, fs;
		SinCos(rotate * T(0.5), fs, fc);

		if (Equal<T>(LengthSq(n), 0))
		{
			return Quaternion_T<T>(fs, fs, fs, fc);
		} 

		return Quaternion(Normalize(n) * fs, fc);
	}

	template Quaternion Slerp(const Quaternion& p1, const Quaternion& p2, float ft);
	template <typename T>
	Quaternion_T<T> Slerp(const Quaternion_T<T> & p1, const Quaternion_T<T> & p2, T ft)
	{
		// 计算2个四元数夹角cos值
		T fCosOmega = Dot(p1, p2);
		// 如果点乘尾负，则反转一个四元数已取得短的4D弧
		T dir = T(1);
		if (fCosOmega < 0.0f)
		{
			dir = -T(-1);
			fCosOmega = -fCosOmega;
		}

		T k1, k2;
		if (fCosOmega < T(1) - std::numeric_limits<T>::epsilon())
		{
			// 用公式sin^2(omega) + con^2(omega) = 1
			T fSinOmege = Sqrt(1.0f - fCosOmega * fCosOmega);
			// 通过sin和con求出角度
			T fOmega = Atan2(fSinOmege, fCosOmega);
			// 为了后面转乘法
			T fOneOverSinOmega = 1.0f / fSinOmege;
			// 计算插值变量
			k1 = Sin((1.0f - ft) * fOmega) * fOneOverSinOmega;
			k2 = Sin(ft * fOmega) * fOneOverSinOmega;
		}
		else
		{
			// sin(Omega)非常小用线性插值
			k1 = T(1) - ft;
			k2 = ft;
		}

		// 插值
		return p1 * k1 * dir + p2 * k2;
	}

	template Quaternion Squad(const Quaternion& q1, const Quaternion& a, const Quaternion& b, const Quaternion& c, float ft);
	template <typename T>
	Quaternion Squad(const Quaternion_T<T> & q1, const Quaternion_T<T> & a, const Quaternion_T<T> & b, const Quaternion_T<T> & c, T ft)
	{
		return Slerp(Slerp(q1, c, ft), Slerp(a, b, ft), T(2) * ft * (1 - ft));
	}

	template void decompose(float3& scale, Quaternion& rot, float3& trans, float4x4 const & rhs) noexcept;
	template <typename T>
	void decompose(Vector_T<T, 3>& scale, Quaternion_T<T>& rot, Vector_T<T, 3>& trans, Matrix4_T<T> const & rhs) noexcept
	{
		scale.x() = Sqrt(rhs(0, 0) * rhs(0, 0) + rhs(0, 1) * rhs(0, 1) + rhs(0, 2) * rhs(0, 2));
		scale.y() = Sqrt(rhs(1, 0) * rhs(1, 0) + rhs(1, 1) * rhs(1, 1) + rhs(1, 2) * rhs(1, 2));
		scale.z() = Sqrt(rhs(2, 0) * rhs(2, 0) + rhs(2, 1) * rhs(2, 1) + rhs(2, 2) * rhs(2, 2));

		trans = Vector_T<T, 3>(rhs(3, 0), rhs(3, 1), rhs(3, 2));
		Matrix4_T<T> rot_mat;
		rot_mat(0, 0) = rhs(0, 0) / scale.x();
		rot_mat(0, 1) = rhs(0, 1) / scale.x();
		rot_mat(0, 2) = rhs(0, 2) / scale.x();
		rot_mat(0, 3) = 0;
		rot_mat(1, 0) = rhs(1, 0) / scale.y();
		rot_mat(1, 1) = rhs(1, 1) / scale.y();
		rot_mat(1, 2) = rhs(1, 2) / scale.y();
		rot_mat(1, 3) = 0;
		rot_mat(2, 0) = rhs(2, 0) / scale.z();
		rot_mat(2, 1) = rhs(2, 1) / scale.z();
		rot_mat(2, 2) = rhs(2, 2) / scale.z();
		rot_mat(2, 3) = 0;
		rot_mat(3, 0) = 0;
		rot_mat(3, 1) = 0;
		rot_mat(3, 2) = 0;
		rot_mat(3, 3) = 1;
		rot = ToQuaternion(rot_mat);
	}

	// From http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/index.htm
	template void to_yaw_pitch_roll(float& yaw, float& pitch, float& roll, Quaternion const & quat) noexcept;
	template <typename T>
	void to_yaw_pitch_roll(T& yaw, T& pitch, T& roll, Quaternion_T<T> const & quat) noexcept
	{
		T sqx = quat.x() * quat.x();
		T sqy = quat.y() * quat.y();
		T sqz = quat.z() * quat.z();
		T sqw = quat.w() * quat.w();
		T unit = sqx + sqy + sqz + sqw;
		T test = quat.w() * quat.x() + quat.y() * quat.z();
		if (test > T(0.499) * unit)
		{
			// singularity at north pole
			yaw = 2 * Atan2(quat.z(), quat.w());
			pitch = PI / 2;
			roll = 0;
		}
		else
		{
			if (test < -T(0.499) * unit)
			{
				// singularity at south pole
				yaw = -2 * Atan2(quat.z(), quat.w());
				pitch = -PI / 2;
				roll = 0;
			}
			else
			{
				yaw = Atan2(2 * (quat.y() * quat.w() - quat.x() * quat.z()), -sqx - sqy + sqz + sqw);
				pitch = Asin(2 * test / unit);
				roll = Atan2(2 * (quat.z() * quat.w() - quat.x() * quat.y()), -sqx + sqy - sqz + sqw);
			}
		}
	}

	template float3 transform_quat(float3 const & v, Quaternion const & quat) noexcept;
	template <typename T>
	Vector_T<T, 3> transform_quat(Vector_T<T, 3> const & v, Quaternion_T<T> const & quat) noexcept
	{
		return v + Cross(quat.GetV(), Cross(quat.GetV(), v) + quat.w() * v) * T(2);
	}
}

