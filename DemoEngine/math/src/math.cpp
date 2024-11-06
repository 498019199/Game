#include <math/math.h>

namespace MathWorker
{
    float sin(float x) noexcept
    {
        return std::sin(x);
    }

    float cos(float x) noexcept
    {
        return std::cos(x);
    }

    // 线性颜色值转换为 sRGB 颜色值的操作
    // 线性颜色空间更适合于计算和物理模拟，而 sRGB 空间则是常见的显示设备所使用的颜色空间。
    // 根据线性颜色值的大小，使用不同的公式将其转换为 sRGB 颜色值。
    float linear_to_srgb(float linear) noexcept
    {
        if (linear < 0.0031308f)
        {
            return 12.92f * linear;
        }
        else
        {
            float const ALPHA = 0.055f;
            return (1 + ALPHA) * pow(linear, 1 / 2.4f) - ALPHA;
        }
    }

    float srgb_to_linear(float srgb) noexcept
    {
        if (srgb < 0.04045f)
        {
            return srgb / 12.92f;
        }
        else
        {
            float const ALPHA = 0.055f;
            return pow((srgb + ALPHA) / (1 + ALPHA), 2.4f);
        }
    }

    void SinCos(float fAnglel, float& X, float&Y)
	{
        X = std::sin(fAnglel);
        Y = std::cos(fAnglel);
	}

    template bool IsEqual(float X, float Y);
    template bool IsEqual(double X, double Y);
    template <typename T>
    bool IsEqual(T X, T Y)
    {
        return std::abs(X - Y) < std::numeric_limits<T>::epsilon();
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
        fni.f = number;                                      // evil floating point bit level hacking
        fni.i = 0x5f375a86 - (fni.i >> 1);                   // what the fuck?
        fni.f = fni.f * (threehalfs - (x2 * fni.f * fni.f)); // 1st iteration
        fni.f = fni.f * (threehalfs - (x2 * fni.f * fni.f)); // 2nd iteration, this can be removed

        return fni.f;
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

    template int32_t Dot(const int2 & lhs, const int2 & rhs) noexcept;
    template int32_t Dot(const int3 & lhs, const int3 & rhs) noexcept;
    template int32_t Dot(const int4 & lhs, const int4 & rhs) noexcept;
    template uint32_t Dot(const uint2 & lhs, const uint2 & rhs) noexcept;
    template uint32_t Dot(const uint3 & lhs, const uint3 & rhs) noexcept;
    template uint32_t Dot(const uint4 & lhs, const uint4 & rhs) noexcept;
    template float Dot(const float2 & lhs, const float2 & rhs) noexcept;
    template float Dot(const float3 & lhs, const float3 & rhs) noexcept;
    template float Dot(const float4 & lhs, const float4 & rhs) noexcept;
	template float Dot(const quater & lhs, const quater & rhs) noexcept;
    template<typename T>
    typename T::value_type Dot(const T & lhs, const T & rhs) noexcept
    {
        return MathHelper::dot_helper < typename T::value_type,
            T::elem_num> ::Do(&lhs[0], &rhs[0]);
    }

    template int32_t LengthSq(const int2 & rhs) noexcept;
    template int32_t LengthSq(const int3 & rhs) noexcept;
    template int32_t LengthSq(const int4 & rhs) noexcept;
    template uint32_t LengthSq(const uint2 & rhs) noexcept;
    template uint32_t LengthSq(const uint3 & rhs) noexcept;
    template uint32_t LengthSq(const uint4 & rhs) noexcept;
    template float LengthSq(const float2 & rhs) noexcept;
    template float LengthSq(const float3 & rhs) noexcept;
    template float LengthSq(const float4 & rhs) noexcept;
	template float LengthSq(const quater & rhs) noexcept;
    template<typename T>
    typename T::value_type LengthSq(const T & rhs) noexcept
    {
        return Dot(rhs, rhs);
    }

    template float Length(const float2 & rhs) noexcept;
    template float Length(const float3 & rhs) noexcept;
    template float Length(const float4 & rhs) noexcept;
	template float Length(const quater & rhs) noexcept;
    template<typename T>
    typename T::value_type Length(const T & rhs) noexcept
    {
        return static_cast<T::value_type>(std::sqrt(LengthSq(rhs)));
    }

	template float2 Normalize(const float2 & rhs) noexcept;
	template float3 Normalize(const float3 & rhs) noexcept;
	template float4 Normalize(const float4 & rhs) noexcept;
	template quater Normalize(const quater & rhs) noexcept;
	template<typename T>
	T Normalize(const T & rhs) noexcept
	{
		typename T::value_type tmp = 
            RecipSqrt(
                LengthSq(rhs));
		return rhs * tmp;
	}

    template int2 Lerp(const int2 &lsh, const int2 &rhs, float s);
    template int3 Lerp(const int3 &lsh, const int3 &rhs, float s);
    template int4 Lerp(const int4 &lsh, const int4 &rhs, float s);
    template uint2 Lerp(const uint2 &lsh, const uint2 &rhs, float s);
    template uint3 Lerp(const uint3 &lsh, const uint3 &rhs, float s);
    template uint4 Lerp(const uint4 &lsh, const uint4 &rhs, float s);
    template float2 Lerp(const float2 &lsh, const float2 &rhs, float s);
    template float3 Lerp(const float3 &lsh, const float3 &rhs, float s);
    template float4 Lerp(const float4 &lsh, const float4 &rhs, float s);
    template <typename T>
    T Lerp(const T &lhs, const T &rhs, float s) noexcept
    {
        return (lhs + (rhs - lhs) * s);
    }

    template int Distance(const int2 & lhs, const int2 & rhs) noexcept;
    template float Distance(const float2 & lhs, const float2 & rhs) noexcept;
    template<typename T>
    T Distance(const Vector_T<T, 2>& lhs, const Vector_T<T, 2>& rhs) noexcept
    {
        Vector_T<T, 2> tmp(lhs.x() - rhs.x(), lhs.y() - rhs.y());
        return Length(tmp);
    }
    
    template int Distance(const int3 & lhs, const int3 & rhs) noexcept;
    template float Distance(const float3 & lhs, const float3 & rhs) noexcept;
    template<typename T>
    T Distance(const Vector_T<T, 3>& lhs, const Vector_T<T, 3>& rhs) noexcept
    {
        Vector_T<T, 3> tmp(lhs.x() - rhs.x(), lhs.y() - rhs.y(), lhs.z() - rhs.z());
        return Length(tmp);
    }

    template float Angle(const float2 &lsh, const float2 &rhs);
    template float Angle(const float3 &lsh, const float3 &rhs);
    template float Angle(const float4 &lsh, const float4 &rhs);
    template <typename T>
    typename T::value_type Angle(const T &lsh, const T &rsh)
    {
        typename T::value_type xn = Length(lsh);
        typename T::value_type yn = Length(rsh);
        typename T::value_type xyn = xn * yn;
        typename T::value_type angle = std::acos((lsh | rsh) / xyn);
        return Rad2Deg(angle);
    }


    template float4x4 MatrixMove(float X, float Y, float Z);
    template<typename T>
    Matrix4_T<T> MatrixMove(T X, T Y, T Z)
    {
        return Matrix4_T<T>(
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            X, Y, Z, 1);
    }

    template float4x4 MatrixMove(const float3& Move);
    template<typename T>
    Matrix4_T<T> MatrixMove(const Vector_T<T, 3> &Move)
    {
        return MatrixMove(Move.x(), Move.y(), Move.z());
    }

    template float4x4 MatrixScale(float X, float Y, float Z);
    template<typename T>
    Matrix4_T<T> MatrixScale(T X, T Y, T Z)
    {
        return Matrix4_T<float>(
            X, 0, 0, 0,
            0, Y, 0, 0,
            0, 0, Z, 0,
            0, 0, 0, 1);
    }

    template float4x4 MatrixScale(const float3& Scale);
    template<typename T>
    Matrix4_T<T> MatrixScale(const Vector_T<T, 3> &Scale)
    {
        return MatrixScale(Scale.x(), Scale.y(), Scale.z());
    }

    template float4x4 MatrixRotateX(float Angle);
    template<typename T>
    Matrix4_T<T> MatrixRotateX(T Angle)
    {
        T fs, fc;
        SinCos(Angle, fs, fc);
        return Matrix4_T<T>(
            1, 0,   0,  0,
            0, fc,  fs, 0,
            0, -fs, fc, 0,
            0, 0,   0,  1);
    }

    template float4x4 MatrixRotateY(float Angle);
    template<typename T>
    Matrix4_T<T> MatrixRotateY(T Angle)
    {
        T fs, fc;
        SinCos(Angle, fs, fc);
        return Matrix4_T<T>(
            fc, 0, -fs, 0,
            0, 1, 0, 0,
            fs, 0, fc, 0,
            0, 0, 0, 1);
    }

    template float4x4 MatrixRotateZ(float Angle);
    template<typename T>
    Matrix4_T<T> MatrixRotateZ(T Angle)
    {
        T fs, fc;
        SinCos(Angle, fs, fc);
        return Matrix4_T<T>(
            fc, fs, 0, 0,
            -fs, fc, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1);
    }

    template float4x4 MatrixRotate(const float3& n, float Angle);
    template<typename T>
    Matrix4_T<T> MatrixRotate(const Vector_T<T, 3>& n, T Angle)
    {
        T fs = 0.0f, fc = 0.0f;
        SinCos(Angle, fs, fc);
        Vector_T<T, 3> v(n.x(), n.y(), n.z());
        v = Normalize(v);

        T a = 1.0f - fc;
        T ax = a * v.x();
        T ay = a * v.y();
        T az = a * v.z();

        Matrix4_T<T> matrix(Matrix4_T<T>::Identity());
        matrix(0, 0) = v.x() * ax + fc;
        matrix(0, 1) = v.x() * ay + v.z() * fs;
        matrix(0, 2) = v.x() * az - v.y() * fs;
        matrix(1, 0) = v.x() * ay - v.z() * fs;
        matrix(1, 1) = v.y() * ay + fc;
        matrix(1, 2) = v.y() * az + v.x() * fs;
        matrix(2, 0) = v.x() * az + v.y() * fs;
        matrix(2, 1) = v.y() * az - v.x() * fs;
        matrix(2, 2) = v.z() * az + fc;
        return matrix;
    }

    template float4x4 Mul(const float4x4& lhs, const float4x4& rhs);
    template<typename T>
    Matrix4_T<T> Mul(const Matrix4_T<T> &lhs, const Matrix4_T<T> &rhs)
    {
        return Matrix4_T<T>(
            lhs(0, 0) * rhs(0, 0) + lhs(0, 1) * rhs(1, 0) + lhs(0, 2) * rhs(2, 0) + lhs(0, 3) * rhs(3, 0),
            lhs(0, 0) * rhs(0, 1) + lhs(0, 1) * rhs(1, 1) + lhs(0, 2) * rhs(2, 1) + lhs(0, 3) * rhs(3, 1),
            lhs(0, 0) * rhs(0, 2) + lhs(0, 1) * rhs(1, 2) + lhs(0, 2) * rhs(2, 2) + lhs(0, 3) * rhs(3, 2),
            lhs(0, 0) * rhs(0, 3) + lhs(0, 1) * rhs(1, 3) + lhs(0, 2) * rhs(2, 3) + lhs(0, 3) * rhs(3, 3),
            lhs(1, 0) * rhs(0, 0) + lhs(1, 1) * rhs(1, 0) + lhs(1, 2) * rhs(2, 0) + lhs(1, 3) * rhs(3, 0),
            lhs(1, 0) * rhs(0, 1) + lhs(1, 1) * rhs(1, 1) + lhs(1, 2) * rhs(2, 1) + lhs(1, 3) * rhs(3, 1),
            lhs(1, 0) * rhs(0, 2) + lhs(1, 1) * rhs(1, 2) + lhs(1, 2) * rhs(2, 2) + lhs(1, 3) * rhs(3, 2),
            lhs(1, 0) * rhs(0, 3) + lhs(1, 1) * rhs(1, 3) + lhs(1, 2) * rhs(2, 3) + lhs(1, 3) * rhs(3, 3),
            lhs(2, 0) * rhs(0, 0) + lhs(2, 1) * rhs(1, 0) + lhs(2, 2) * rhs(2, 0) + lhs(2, 3) * rhs(3, 0),
            lhs(2, 0) * rhs(0, 1) + lhs(2, 1) * rhs(1, 1) + lhs(2, 2) * rhs(2, 1) + lhs(2, 3) * rhs(3, 1),
            lhs(2, 0) * rhs(0, 2) + lhs(2, 1) * rhs(1, 2) + lhs(2, 2) * rhs(2, 2) + lhs(2, 3) * rhs(3, 2),
            lhs(2, 0) * rhs(0, 3) + lhs(2, 1) * rhs(1, 3) + lhs(2, 2) * rhs(2, 3) + lhs(2, 3) * rhs(3, 3),
            lhs(3, 0) * rhs(0, 0) + lhs(3, 1) * rhs(1, 0) + lhs(3, 2) * rhs(2, 0) + lhs(3, 3) * rhs(3, 0),
            lhs(3, 0) * rhs(0, 1) + lhs(3, 1) * rhs(1, 1) + lhs(3, 2) * rhs(2, 1) + lhs(3, 3) * rhs(3, 1),
            lhs(3, 0) * rhs(0, 2) + lhs(3, 1) * rhs(1, 2) + lhs(3, 2) * rhs(2, 2) + lhs(3, 3) * rhs(3, 2),
            lhs(3, 0) * rhs(0, 3) + lhs(3, 1) * rhs(1, 3) + lhs(3, 2) * rhs(2, 3) + lhs(3, 3) * rhs(3, 3));
    }

    // 1*4 mul 4*4
    template float4 Mul(const float4& a, const float4x4& mat);
    template<typename T>
    Vector_T<T, 4> Mul(const Vector_T<T, 4> &a, const Matrix4_T<T> &mat)
    {
		return Vector_T<T, 4>(a.x() * mat(0, 0) + a.y() * mat(1, 0) + a.z() * mat(2, 0) + a.w() * mat(3, 0),
                                a.x() * mat(0, 1) + a.y() * mat(1, 1) + a.z() * mat(2, 1) + a.w() * mat(3, 1),
                                a.x() * mat(0, 2) + a.y() * mat(1, 2) + a.z() * mat(2, 2) + a.w() * mat(3, 2),
                                a.x() * mat(0, 3) + a.y() * mat(1, 3) + a.z() * mat(2, 3) + a.w() * mat(3, 3));
    }

    template float4x4 Transpose(const float4x4& mat);
    template<typename T>
    Matrix4_T<T> Transpose(const Matrix4_T<T> &mat)
    {
        return Matrix4_T<T>(mat(0, 0), mat(1, 0), mat(2, 0), mat(3, 0),
                      mat(0, 1), mat(1, 1), mat(2, 1), mat(3, 1),
                      mat(0, 2), mat(1, 2), mat(2, 2), mat(3, 2),
                      mat(0, 3), mat(1, 3), mat(2, 3), mat(3, 3));
    }

    // 矩阵的行列式
    template float Determinant(const float4x4& mat);
    template<typename T>
    T Determinant(const Matrix4_T<T> &mat)
    {
        const float _3142_3241(mat(2, 0) * mat(3, 1) - mat(2, 1) * mat(3, 0));
        const float _3143_3341(mat(2, 0) * mat(3, 2) - mat(2, 2) * mat(3, 0));
        const float _3144_3441(mat(2, 0) * mat(3, 3) - mat(2, 3) * mat(3, 0));
        const float _3243_3342(mat(2, 1) * mat(3, 2) - mat(2, 2) * mat(3, 1));
        const float _3244_3442(mat(2, 1) * mat(3, 3) - mat(2, 3) * mat(3, 1));
        const float _3344_3443(mat(2, 2) * mat(3, 3) - mat(2, 3) * mat(3, 2));

        return mat(0, 0) * (mat(1, 1) * _3344_3443 - mat(1, 2) * _3244_3442 + mat(1, 3) * _3243_3342) - 
        mat(0, 1) * (mat(1, 0) * _3344_3443 - mat(1, 2) * _3144_3441 + mat(1, 3) * _3143_3341) + 
        mat(0, 2) * (mat(1, 0) * _3244_3442 - mat(1, 1) * _3144_3441 + mat(1, 3) * _3142_3241) - 
        mat(0, 3) * (mat(1, 0) * _3243_3342 - mat(1, 1) * _3143_3341 + mat(1, 2) * _3142_3241);
    }

    // 矩阵的逆
    template float4x4 MatrixInverse(const float4x4& mat);
    template<typename T>
    Matrix4_T<T> MatrixInverse(const Matrix4_T<T>& mat)
    {
        const float _2132_2231(mat(1, 0) * mat(2, 1) - mat(1, 1) * mat(2, 0));
        const float _2133_2331(mat(1, 0) * mat(2, 2) - mat(1, 2) * mat(2, 0));
        const float _2134_2431(mat(1, 0) * mat(2, 3) - mat(1, 3) * mat(2, 0));
        const float _2142_2241(mat(1, 0) * mat(3, 1) - mat(1, 1) * mat(3, 0));
        const float _2143_2341(mat(1, 0) * mat(3, 2) - mat(1, 2) * mat(3, 0));
        const float _2144_2441(mat(1, 0) * mat(3, 3) - mat(1, 3) * mat(3, 0));
        const float _2233_2332(mat(1, 1) * mat(2, 2) - mat(1, 2) * mat(2, 1));
        const float _2234_2432(mat(1, 1) * mat(2, 3) - mat(1, 3) * mat(2, 1));
        const float _2243_2342(mat(1, 1) * mat(3, 2) - mat(1, 2) * mat(3, 1));
        const float _2244_2442(mat(1, 1) * mat(3, 3) - mat(1, 3) * mat(3, 1));
        const float _2334_2433(mat(1, 2) * mat(2, 3) - mat(1, 3) * mat(2, 2));
        const float _2344_2443(mat(1, 2) * mat(3, 3) - mat(1, 3) * mat(3, 2));
        const float _3142_3241(mat(2, 0) * mat(3, 1) - mat(2, 1) * mat(3, 0));
        const float _3143_3341(mat(2, 0) * mat(3, 2) - mat(2, 2) * mat(3, 0));
        const float _3144_3441(mat(2, 0) * mat(3, 3) - mat(2, 3) * mat(3, 0));
        const float _3243_3342(mat(2, 1) * mat(3, 2) - mat(2, 2) * mat(3, 1));
        const float _3244_3442(mat(2, 1) * mat(3, 3) - mat(2, 3) * mat(3, 1));
        const float _3344_3443(mat(2, 2) * mat(3, 3) - mat(2, 3) * mat(3, 2));

        // 行列式的值
        const float det(Determinant(mat));
        if (IsEqual<float>(det, 0))
        {
            return mat;
        }
        else
        {
            // 标准伴随矩阵的转置 / 行列式的值
            float invDet(float(1) / det);
            return Matrix4_T<T>(
                +invDet * (mat(1, 1) * _3344_3443 - mat(1, 2) * _3244_3442 + mat(1, 3) * _3243_3342), // c11
                -invDet * (mat(0, 1) * _3344_3443 - mat(0, 2) * _3244_3442 + mat(0, 3) * _3243_3342), // c21
                +invDet * (mat(0, 1) * _2344_2443 - mat(0, 2) * _2244_2442 + mat(0, 3) * _2243_2342), // c31
                -invDet * (mat(0, 1) * _2334_2433 - mat(0, 2) * _2234_2432 + mat(0, 3) * _2233_2332), // c41

                -invDet * (mat(1, 0) * _3344_3443 - mat(1, 2) * _3144_3441 + mat(1, 3) * _3143_3341), // c12
                +invDet * (mat(0, 0) * _3344_3443 - mat(0, 2) * _3144_3441 + mat(0, 3) * _3143_3341), // c22
                -invDet * (mat(0, 0) * _2344_2443 - mat(0, 2) * _2144_2441 + mat(0, 3) * _2143_2341), // c32
                +invDet * (mat(0, 0) * _2334_2433 - mat(0, 2) * _2134_2431 + mat(0, 3) * _2133_2331), // c42

                +invDet * (mat(1, 0) * _3244_3442 - mat(1, 1) * _3144_3441 + mat(1, 3) * _3142_3241), // c13
                -invDet * (mat(0, 0) * _3244_3442 - mat(0, 1) * _3144_3441 + mat(0, 3) * _3142_3241), // c23
                +invDet * (mat(0, 0) * _2244_2442 - mat(0, 1) * _2144_2441 + mat(0, 3) * _2142_2241), // c33
                -invDet * (mat(0, 0) * _2234_2432 - mat(0, 1) * _2134_2431 + mat(0, 3) * _2132_2231), // c43

                -invDet * (mat(1, 0) * _3243_3342 - mat(1, 1) * _3143_3341 + mat(1, 2) * _3142_3241),  // c14
                +invDet * (mat(0, 0) * _3243_3342 - mat(0, 1) * _3143_3341 + mat(0, 2) * _3142_3241),  // c24
                -invDet * (mat(0, 0) * _2243_2342 - mat(0, 1) * _2143_2341 + mat(0, 2) * _2142_2241),  // c34
                +invDet * (mat(0, 0) * _2233_2332 - mat(0, 1) * _2133_2331 + mat(0, 2) * _2132_2231)); // c44
        }
    }

    template float4x4 LHToRH(const float4x4& float4x4) noexcept;
    template<typename T>
    Matrix4_T<T> LHToRH(const Matrix4_T<T>& rhs) noexcept
    {
        Matrix4_T<T> ret = rhs;
        ret(2, 0) = -ret(2, 0);
        ret(2, 1) = -ret(2, 1);
        ret(2, 2) = -ret(2, 2);
        ret(2, 3) = -ret(2, 3);
        return ret;
    }

    // 视口为中心的正交投影矩阵
	float4x4 OrthoLH(float w, float h, float nearPlane, float farPlane);
    template<typename T>
	Matrix4_T<T> OrthoLH(T w, T h, T nearPlane, T farPlane)
    {
        const T w_2(w / 2); // w = right - left ，left = -right
	    const T h_2(h / 2); // h = top - bottom ，top = -bottom
        return OrthoOffCenterLH(-w_2, w_2, -h_2, h_2, farPlane, nearPlane);            
    }

    // dx->[-1,1][-1,1][0,1]，selected
    // openGL->[-1,1][-1,1][-1,1]
	float4x4 OrthoOffCenterLH(float left, float right, float bottom, float top, float farPlane, float nearPlane);
    template<typename T>
	Matrix4_T<T> OrthoOffCenterLH(T left, T right, T bottom, T top, T farPlane, T nearPlane)
    {
        const T q(1.f / (farPlane - nearPlane));
		const T invWidth(1.f / (right - left));
		const T invHeight(1.f / (top - bottom));
        return Matrix4_T<T>(
            invWidth + invWidth,    0,                          0,              0,
            0,                      invHeight + invHeight,      0,              0,
            0,                      0,                          q,              0,
            -(left + right)/invWidth,   -(top + bottom)/invHeight, -nearPlane/q,    1);        
    }

    template<typename T>
	Matrix4_T<T> OrthoRH(T w, T h, T farPlane, T nearPlane)
	{
		const T w_2(w / 2); // w = right - left ，left = -right
	    const T h_2(h / 2); // h = top - bottom ，top = -bottom
        return OrthoOffCenterRH(-w_2, w_2, -h_2, h_2, farPlane, nearPlane);            

	}

    template<typename T>
	Matrix4_T<T> OrthoOffCenterRH(T left, T right, T bottom, T top, T farPlane, T nearPlane)
	{
		const T q(1.f / (farPlane - nearPlane));
		const T invWidth(1.f / (right - left));
		const T invHeight(1.f / (top - bottom));
        return Matrix4_T<T>(
            invWidth + invWidth,    0,                          0,              0,
            0,                      invHeight + invHeight,      0,              0,
            0,                      0,                          q,              0,
            -(left + right)/invWidth,   -(top + bottom)/invHeight, -(farPlane + nearPlane)/q,    1);        
  
	}

    // https://learn.microsoft.com/en-us/previous-versions/windows/desktop/bb281711(v=vs.85)
	float4x4 LookAtRH(const float3& Eye, const float3& At, const float3& Up);
    template<typename T>
	Matrix4_T<T> LookAtRH(const Vector_T<T, 3>& Eye, const Vector_T<T, 3>& At, const Vector_T<T, 3>& Up)
    {
        Vector_T<T, 3> ZAxis = Normalize(Eye - At);// 朝-z方向
        Vector_T<T, 3> XAxis = Normalize(Up ^ ZAxis);
        Vector_T<T, 3> YAxis = ZAxis ^ XAxis;
        return Matrix4_T<T>(
            XAxis.x(),      YAxis.x(),      ZAxis.x(),      0,
            XAxis.y(),      YAxis.y(),      ZAxis.y(),      0,
            XAxis.z(),      YAxis.z(),      ZAxis.z(),      0,
            -XAxis | Eye,   -YAxis | Eye,   -ZAxis | Eye,   1);        
    }

    // https://learn.microsoft.com/en-us/previous-versions/windows/desktop/bb281710(v=vs.85)
    // Eye 是摄像机位置，At 是摄像机朝向方向，Up 摄像机向上方向
	float4x4 LookAtLH(const float3& Eye, const float3& At, const float3& Up);
    template<typename T>
	Matrix4_T<T> LookAtLH(const Vector_T<T, 3>& Eye, const Vector_T<T, 3>& At, const Vector_T<T, 3>& Up)
    {
        Vector_T<T, 3> ZAxis = Normalize(At - Eye);
        Vector_T<T, 3> XAxis = Normalize(Up ^ ZAxis);
        Vector_T<T, 3> YAxis = ZAxis ^ XAxis;
        return Matrix4_T<T>(
            XAxis.x(),      YAxis.x(),      ZAxis.x(),      0,
            XAxis.y(),      YAxis.y(),      ZAxis.y(),      0,
            XAxis.z(),      YAxis.z(),      ZAxis.z(),      0,
            -XAxis | Eye,   -YAxis | Eye,   -ZAxis | Eye,   1);        
    }

    template float4x4 PerspectiveLH(float w, float h, float Near, float Far);
    template<typename T>
    Matrix4_T<T> PerspectiveLH(T w, T h, T Near, T Far)
    {
        const T  q(Far / (Far - Near));
        const T  near2(Near + Near);

        return Matrix4_T<T>(
            near2 / w,	0,				0,				0,
            0,				near2 / h,	0,				0,
            0,				0,			q,			    1,
            0,				0,			-Near * q,      0);    
    }

    template float4x4 PerspectiveFovLH(float Fov, float Aspect, float Near, float Far);
    template<typename T>
	Matrix4_T<T> PerspectiveFovLH(T Fov, T Aspect, T Near, T Far)
    {
        const T  h(T(1) / std::tan(Fov / 2));
        const T  w(h / Aspect);
        const T  q(Far / (Far - Near));

        return Matrix4_T<T>(
            w,		0,		0,		   0,
            0,		h,		0,		   0,
            0,		0,		q,		   1,
            0,		0,		-Near * q, 0);      
    }

    template float4x4 PerspectiveOffCenterLH(float left, float right, float bottom, float top, float farPlane, float nearPlane);
    template<typename T>
    Matrix4_T<T> PerspectiveOffCenterLH(T left, T right, T bottom, T top, T farPlane, T nearPlane)
    {
        const T q(farPlane / (farPlane - nearPlane));
        const T near2(nearPlane + nearPlane);
        const T invWidth(T(1) / (right - left));
        const T invHeight(T(1) / (top - bottom));

        return Matrix4_T<T>(
            near2 * invWidth,			0,								0,				0,
            0,							near2 * invHeight,				0,				0,
            -(left + right) * invWidth,	-(top + bottom) * invHeight,	q,				1,
            0,							0,								-nearPlane * q, 0);
    }

    template float4x4 PerspectiveRH(float w, float h, float Near, float Far);
    template<typename T>
    Matrix4_T<T> PerspectiveRH(T w, T h, T Near, T Far)
    {
        return LHToRH(PerspectiveLH(w, h, Near, Far));          
    }
    
    template float4x4 PerspectiveOffCenterRH(float left, float right, float bottom, float top, float farPlane, float nearPlane);
    template<typename T>
	Matrix4_T<T> PerspectiveOffCenterRH(T left, T right, T bottom, T top, T farPlane, T nearPlane)
    {
        return LHToRH(PerspectiveOffCenterLH(left, right, bottom, top, farPlane, nearPlane));        
    }

    template float4x4 PerspectiveFovRH(float Fov, float Aspect, float Near, float Far);
	template<typename T>
	Matrix4_T<T> PerspectiveFovRH(T Fov, T Aspect, T Near, T Far)
    {
        const T  h(T(1) / std::tan(Fov / 2));
        const T  w(h / Aspect);
        const T  q(Far - Near);

        return Matrix4_T<T>(
            w,		0,		0,		                    0,
            0,		h,		0,		                    0,
            0,		0,		-(Far + Near) / q,		   -1,
            0,		0,		-(2 * Far * Near) / q,      0); 
    }

    template quater Mul(const quater& lhs, const quater& rhs) noexcept;
    template<typename T>
	Quaternion_T<T> Mul(const Quaternion_T<T>& lhs, const Quaternion_T<T>& rhs) noexcept
	{
		return Quaternion_T<T>(
			lhs.x() * rhs.w() - lhs.y() * rhs.z() + lhs.z() * rhs.y() + lhs.w() * rhs.x(),
			lhs.x() * rhs.z() + lhs.y() * rhs.w() - lhs.z() * rhs.x() + lhs.w() * rhs.y(),
			lhs.y() * rhs.x() - lhs.x() * rhs.y() + lhs.z() * rhs.w() + lhs.w() * rhs.z(),
			lhs.w() * rhs.w() - lhs.x() * rhs.x() - lhs.y() * rhs.y() - lhs.z() * rhs.z());
	}

    template quater Conjugate(quater const & rhs) noexcept;
	template <typename T>
	Quaternion_T<T> Conjugate(Quaternion_T<T> const & rhs) noexcept
	{
		return Quaternion_T<T>(-rhs.x(), -rhs.y(), -rhs.z(), rhs.w());
	}

	template quater Inverse(const quater& rhs) noexcept;
	template <typename T>
	Quaternion_T<T> Inverse(const Quaternion_T<T>& rhs) noexcept
	{
		T var(T(1) / Length(rhs));
		return Quaternion_T<T>(-rhs.x() * var, -rhs.y() * var, -rhs.z() * var, rhs.w() * var);
	}

    float4x4 ToMatrix(const quater &quat)
    {
        // calculate coefficients
        const float x2(quat.x() + quat.x());
        const float y2(quat.y() + quat.y());
        const float z2(quat.z() + quat.z());

        const float xx2(quat.x() * x2), xy2(quat.x() * y2), xz2(quat.x() * z2);
        const float yy2(quat.y() * y2), yz2(quat.y() * z2), zz2(quat.z() * z2);
        const float wx2(quat.w() * x2), wy2(quat.w() * y2), wz2(quat.w() * z2);

        return float4x4(
            1 - yy2 - zz2,	xy2 + wz2,		xz2 - wy2,		0,
            xy2 - wz2,		1 - xx2 - zz2,	yz2 + wx2,		0,
            xz2 + wy2,		yz2 - wx2,		1 - xx2 - yy2,	0,
            0,				0,				0,				1);
    }

    float4x4 ToMatrix(const rotator &rot)
    {
        float4x4 rot_x = MatrixRotateX(rot.pitch());
        float4x4 rot_y = MatrixRotateX(rot.yaw());
        float4x4 rot_z = MatrixRotateX(rot.roll());
        return rot_x * rot_y * rot_z;
    }

    quater ToQuaternion(const float4x4 &mat)
    {
        quater quat;
        float s;
        const float tr = mat(0, 0) + mat(1, 1) + mat(2, 2) + 1;

        // check the diagonal
        if (tr > 1)
        {
            s = sqrt(tr);
            quat.w() = s * 0.5f;
            s = 0.5f / s;
            quat.x() = (mat(1, 2) - mat(2, 1)) * s;
            quat.y() = (mat(2, 0) - mat(0, 2)) * s;
            quat.z() = (mat(0, 1) - mat(1, 0)) * s;
        }
        else
        {
            size_t maxi = 0;
            float maxdiag = mat(0, 0);
            for (size_t i = 1; i < 3; ++ i)
            {
                if (mat(i, i) > maxdiag)
                {
                    maxi = i;
                    maxdiag = mat(i, i);
                }
            }

            switch (maxi)
            {
            case 0:
                s = sqrt((mat(0, 0) - (mat(1, 1) + mat(2, 2))) + 1);

                quat.x() = s * 0.5f;

                if (!IsEqual(s, 0.f))
                {
                    s = 0.5f / s;
                }

                quat.w() = (mat(1, 2) - mat(2, 1)) * s;
                quat.y() = (mat(1, 0) + mat(0, 1)) * s;
                quat.z() = (mat(2, 0) + mat(0, 2)) * s;
                break;

            case 1:
                s = sqrt((mat(1, 1) - (mat(2, 2) + mat(0, 0))) + 1);
                quat.y() = s * 0.5f;

                if (!IsEqual(s, 0.f))
                {
                    s = 0.5f / s;
                }

                quat.w() = (mat(2, 0) - mat(0, 2)) * s;
                quat.z() = (mat(2, 1) + mat(1, 2)) * s;
                quat.x() = (mat(0, 1) + mat(1, 0)) * s;
                break;

            case 2:
            default:
                s = sqrt((mat(2, 2) - (mat(0, 0) + mat(1, 1))) + 1);

                quat.z() = s * 0.5f;

                if (!IsEqual(s, 0.f))
                {
                    s = 0.5f / s;
                }

                quat.w() = (mat(0, 1) - mat(1, 0)) * s;
                quat.x() = (mat(0, 2) + mat(2, 0)) * s;
                quat.y() = (mat(1, 2) + mat(2, 1)) * s;
                break;
            }
        }

        return Normalize(quat);
    }

    quater ToQuaternion(const rotator &rot)
    {
        const float angX(rot.pitch() / 2), angY(rot.yaw() / 2), angZ(rot.roll() / 2);
        float sx, sy, sz;
        float cx, cy, cz;
        SinCos(angX, sx, cx);
        SinCos(angY, sy, cy);
        SinCos(angZ, sz, cz);

        return quater(
            sx * cy * cz + cx * sy * sz,
            cx * sy * cz - sx * cy * sz,
            cx * cy * sz - sx * sy * cz,
            sx * sy * sz + cx * cy * cz);
    }

    rotator ToRotator(const float4x4 &mat)
    {
        return rotator();
    }

    // From http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/index.htm
    rotator ToRotator(const quater &quat)
    {
        float sqx = quat.x() * quat.x();
        float sqy = quat.y() * quat.y();
        float sqz = quat.z() * quat.z();
        float sqw = quat.w() * quat.w();
        float unit = sqx + sqy + sqz + sqw;
        float test = quat.w() * quat.x() + quat.y() * quat.z();
        rotator rot;
        if (test > 0.499f * unit)
        {
            // singularity at north pole
            rot.yaw() = 2 * atan2(quat.z(), quat.w());
            rot.pitch() = PI / 2;
            rot.roll() = 0;
        }
        else
        {
            if (test < -(0.499f) * unit)
            {
                // singularity at south pole
                rot.yaw() = -2 * atan2(quat.z(), quat.w());
                rot.pitch() = -PI / 2;
                rot.roll() = 0;
            }
            else
            {
                rot.yaw() = atan2(2 * (quat.y() * quat.w() - quat.x() * quat.z()), -sqx - sqy + sqz + sqw);
                rot.pitch() = asin(2 * test / unit);
                rot.roll() = atan2(2 * (quat.z() * quat.w() - quat.x() * quat.y()), -sqx + sqy - sqz + sqw);
            }
        }
        return rot;
    }

}
