#include <math/math.h>
#include <vector>

namespace RenderWorker
{
    namespace MathWorker
    {
        template int1 abs(const int1 & x) noexcept;
        template int2 abs(const int2 & x) noexcept;
        template int3 abs(const int3 & x) noexcept;
        template int4 abs(const int4 & x) noexcept;
        template float1 abs(const float1 & x) noexcept;
        template float2 abs(const float2 & x) noexcept;
        template float3 abs(const float3 & x) noexcept;
        template float4 abs(const float4 & x) noexcept;
        template <typename T, size_t N>
        Vector_T<T, N> abs(Vector_T<T, N> const & x) noexcept
        {
            Vector_T<T, N> ret;
            for (size_t i = 0; i < N; ++ i)
            {
                ret[i] = MathWorker::abs(x[i]);
            }
            return ret;
        }

        template int1 sgn(int1 const & x) noexcept;
		template int2 sgn(int2 const & x) noexcept;
		template int3 sgn(int3 const & x) noexcept;
		template int4 sgn(int4 const & x) noexcept;
		template float1 sgn(float1 const & x) noexcept;
		template float2 sgn(float2 const & x) noexcept;
		template float3 sgn(float3 const & x) noexcept;
		template float4 sgn(float4 const & x) noexcept;
		template <typename T, size_t N>
		Vector_T<T, N> sgn(Vector_T<T, N> const & x) noexcept
		{
			Vector_T<T, N> ret;
			for (size_t i = 0; i < N; ++ i)
			{
				ret[i] = MathWorker::sgn(x[i]);
			}
			return ret;
		}

        float sin(float x) noexcept
        {
            return std::sin(x);
        }

        float cos(float x) noexcept
        {
            return sin(x + PI / 2);
        }

        
        void sincos(float fAnglel, float& s, float& c) noexcept
        {
            s = sin(fAnglel);
            c = cos(fAnglel);
        }
        
        int32_t SignBit(int32_t x) noexcept
		{
			return (x & 0x80000000U) ? -1 : 1;
		}

		float SignBit(float x) noexcept
		{
			return static_cast<float>(SignBit(std::bit_cast<int32_t>(x)));
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

        float sqrt(float x) noexcept
		{
			return std::sqrt(x);
		}

        // From Quake III. But the magic number is from http://www.lomont.org/Math/Papers/2003/InvSqrt.pdf
        float recip_sqrt(float number) noexcept
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

        template int32_t cross(const int2 & lhs, const int2 & rhs) noexcept;
        template uint32_t cross(const uint2 & lhs, const uint2 & rhs) noexcept;
        template float cross(const float2 & lhs, const float2 & rhs) noexcept;
        template<typename T>
        T cross(const Vector_T<T, 2> & lhs, const Vector_T<T, 2> & rhs) noexcept
        {
            return lhs.x() * rhs.y() - lhs.y() * rhs.x();
        }

        template int3 cross(const int3 & lhs, const int3 & rhs) noexcept;
        template uint3 cross(const uint3 & lhs, const uint3 & rhs) noexcept;
        template float3 cross(const float3 & lhs, const float3 & rhs) noexcept;
        template<typename T>
        Vector_T<T, 3> cross(const Vector_T<T, 3>& lhs, const Vector_T<T, 3>& rhs) noexcept
        {
            return Vector_T<T, 3>((lhs.y() * rhs.z() - lhs.z() * rhs.y()),
                                (lhs.z() * rhs.x() - lhs.x() * rhs.z()),
                                (lhs.x() * rhs.y() - lhs.y() * rhs.x()));
        }

        template int4 cross(const int4 & lhs, const int4 & rhs) noexcept;
        template uint4 cross(const uint4 & lhs, const uint4 & rhs) noexcept;
        template float4 cross(const float4 & lhs, const float4 & rhs) noexcept;
        template<typename T>
        Vector_T<T, 4>
            cross(const Vector_T<T, 4> & lhs, const Vector_T<T, 4> & rhs) noexcept
        {
            return Vector_T<T, 4>((lhs.y() * rhs.z() - lhs.z() * rhs.y()),
                (lhs.z() * rhs.x() - lhs.x() * rhs.z()),
                (lhs.x() * rhs.y() - lhs.y() * rhs.x()),
                1);
        }

        template float3 transform_quat(float3 const & v, quater const & quat) noexcept;
		template <typename T>
		Vector_T<T, 3> transform_quat(Vector_T<T, 3> const & v, Quaternion_T<T> const & quat) noexcept
		{
			return v + cross(quat.v(), cross(quat.v(), v) + quat.w() * v) * T(2);
		}

        template int32_t dot(const int2 & lhs, const int2 & rhs) noexcept;
        template int32_t dot(const int3 & lhs, const int3 & rhs) noexcept;
        template int32_t dot(const int4 & lhs, const int4 & rhs) noexcept;
        template uint32_t dot(const uint2 & lhs, const uint2 & rhs) noexcept;
        template uint32_t dot(const uint3 & lhs, const uint3 & rhs) noexcept;
        template uint32_t dot(const uint4 & lhs, const uint4 & rhs) noexcept;
        template float dot(const float2 & lhs, const float2 & rhs) noexcept;
        template float dot(const float3 & lhs, const float3 & rhs) noexcept;
        template float dot(const float4 & lhs, const float4 & rhs) noexcept;
        template float dot(const quater & lhs, const quater & rhs) noexcept;
        template float dot(const Color& lhs, const Color& rhs) noexcept;
        template<typename T>
        typename T::value_type dot(const T & lhs, const T & rhs) noexcept
        {
            return MathHelper::dot_helper < typename T::value_type,
                T::elem_num> ::Do(&lhs[0], &rhs[0]);
        }

        template int32_t length_sq(const int2 & rhs) noexcept;
        template int32_t length_sq(const int3 & rhs) noexcept;
        template int32_t length_sq(const int4 & rhs) noexcept;
        template uint32_t length_sq(const uint2 & rhs) noexcept;
        template uint32_t length_sq(const uint3 & rhs) noexcept;
        template uint32_t length_sq(const uint4 & rhs) noexcept;
        template float length_sq(const float2 & rhs) noexcept;
        template float length_sq(const float3 & rhs) noexcept;
        template float length_sq(const float4 & rhs) noexcept;
        template float length_sq(const quater & rhs) noexcept;
        template<typename T>
        typename T::value_type length_sq(const T & rhs) noexcept
        {
            return dot(rhs, rhs);
        }

        template float length(const float2 & rhs) noexcept;
        template float length(const float3 & rhs) noexcept;
        template float length(const float4 & rhs) noexcept;
        template float length(const quater & rhs) noexcept;
        template<typename T>
        typename T::value_type length(const T & rhs) noexcept
        {
            return static_cast<T::value_type>(std::sqrt(length_sq(rhs)));
        }

        template float2 normalize(const float2 & rhs) noexcept;
        template float3 normalize(const float3 & rhs) noexcept;
        template float4 normalize(const float4 & rhs) noexcept;
        template quater normalize(const quater & rhs) noexcept;
        template<typename T>
        T normalize(const T & rhs) noexcept
        {
            typename T::value_type tmp = 
                recip_sqrt(
                    length_sq(rhs));
            return rhs * tmp;
        }

        template float lerp(const float &lsh, const float &rhs, float s);
        template float2 lerp(const float2 &lsh, const float2 &rhs, float s);
        template float3 lerp(const float3 &lsh, const float3 &rhs, float s);
        template float4 lerp(const float4 &lsh, const float4 &rhs, float s);
        template Color lerp(const Color &lsh, const Color &rhs, float s);
        template <typename T>
        T lerp(const T &lhs, const T &rhs, float s) noexcept
        {
            return (lhs + (rhs - lhs) * s);
        }

        template int2 maximize(const int2 & lhs, const int2 & rhs) noexcept;
        template int3 maximize(const int3 & lhs, const int3 & rhs) noexcept;
        template int4 maximize(const int4 & lhs, const int4 & rhs) noexcept;
        template uint2 maximize(const uint2 & lhs, const uint2 & rhs) noexcept;
        template uint3 maximize(const uint3 & lhs, const uint3 & rhs) noexcept;
        template uint4 maximize(const uint4 & lhs, const uint4 & rhs) noexcept;
        template float2 maximize(const float2 & lhs, const float2 & rhs) noexcept;
        template float3 maximize(const float3 & lhs, const float3 & rhs) noexcept;
        template float4 maximize(const float4 & lhs, const float4 & rhs) noexcept;
        template <typename T>
        T maximize(const T & lhs, const T & rhs) noexcept
        {
            T ret;
            MathHelper::max_minimize_helper<typename T::value_type, T::elem_num>::DoMax(&ret[0], &lhs[0], &rhs[0]);
            return ret;
        }

        template int2 minimize(const int2 & lhs, const int2 & rhs) noexcept;
        template int3 minimize(const int3 & lhs, const int3 & rhs) noexcept;
        template int4 minimize(const int4 & lhs, const int4 & rhs) noexcept;
        template uint2 minimize(const uint2 & lhs, const uint2 & rhs) noexcept;
        template uint3 minimize(const uint3 & lhs, const uint3 & rhs) noexcept;
        template uint4 minimize(const uint4 & lhs, const uint4 & rhs) noexcept;
        template float2 minimize(const float2 & lhs, const float2 & rhs) noexcept;
        template float3 minimize(const float3 & lhs, const float3 & rhs) noexcept;
        template float4 minimize(const float4 & lhs, const float4 & rhs) noexcept;
        template <typename T>
        T minimize(const T & lhs, const T & rhs) noexcept
        {
            T ret;
            MathHelper::max_minimize_helper<typename T::value_type, T::elem_num>::DoMin(&ret[0], &lhs[0], &rhs[0]);
            return ret;
        }

        template float4 transform(const float2 & v, const float4x4 & mat) noexcept;
        template float4 transform(const float3 & v, const float4x4 & mat) noexcept;
        template float4 transform(const float4 & v, const float4x4 & mat) noexcept;
        template <typename T>
        Vector_T<typename T::value_type, 4> transform(const T & v, const Matrix4_T<typename T::value_type> & mat) noexcept
        {
            return MathHelper::transform_helper<typename T::value_type, T::elem_num>::Do(v, mat);
        }

        template float2 transform_coord(float2 const & v, float4x4 const & mat) noexcept;
		template float3 transform_coord(float3 const & v, float4x4 const & mat) noexcept;
		template <typename T>
		T transform_coord(T const & v, Matrix4_T<typename T::value_type> const & mat) noexcept
		{
			static_assert(T::elem_num < 4, "Must be at most 4D vector.");

			Vector_T<typename T::value_type, 4> temp(MathHelper::transform_helper<typename T::value_type, T::elem_num>::Do(v, mat));
			Vector_T<typename T::value_type, T::elem_num> ret(&temp[0]);
			if (equal(temp.w(), typename T::value_type(0)))
			{
				ret = T::Zero();
			}
			else
			{
				ret /= temp.w();
			}
			return ret;
		}

        template float Angle(const float2 &lsh, const float2 &rhs);
        template float Angle(const float3 &lsh, const float3 &rhs);
        template float Angle(const float4 &lsh, const float4 &rhs);
        template <typename T>
        typename T::value_type Angle(const T &lsh, const T &rsh)
        {
            typename T::value_type xn = length(lsh);
            typename T::value_type yn = length(rsh);
            typename T::value_type xyn = xn * yn;
            typename T::value_type angle = std::acos((lsh | rsh) / xyn);
            return Rad2Deg(angle);
        }


        template float4x4 translation(float X, float Y, float Z);
        template<typename T>
        Matrix4_T<T> translation(T X, T Y, T Z)
        {
            return Matrix4_T<T>(
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                X, Y, Z, 1);
        }

        template float4x4 translation(const float3& Move);
        template<typename T>
        Matrix4_T<T> translation(const Vector_T<T, 3> &Move)
        {
            return translation(Move.x(), Move.y(), Move.z());
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

        template float4x4 rotation_x(float Angle);
        template<typename T>
        Matrix4_T<T> rotation_x(T Angle)
        {
            T fs, fc;
            sincos(Angle, fs, fc);
            return Matrix4_T<T>(
                1, 0,   0,  0,
                0, fc,  fs, 0,
                0, -fs, fc, 0,
                0, 0,   0,  1);
        }

        template float4x4 rotation_y(float Angle);
        template<typename T>
        Matrix4_T<T> rotation_y(T Angle)
        {
            T fs, fc;
            sincos(Angle, fs, fc);
            return Matrix4_T<T>(
                fc, 0, -fs, 0,
                0, 1, 0, 0,
                fs, 0, fc, 0,
                0, 0, 0, 1);
        }

        template float4x4 rotation_z(float Angle);
        template<typename T>
        Matrix4_T<T> rotation_z(T Angle)
        {
            T fs, fc;
            sincos(Angle, fs, fc);
            return Matrix4_T<T>(
                fc, fs, 0, 0,
                -fs, fc, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1);
        }

		template float4x4 rotation_matrix_yaw_pitch_roll(const float& yaw, const float& pitch, const float& roll) noexcept;
		template <typename T>
		Matrix4_T<T> rotation_matrix_yaw_pitch_roll(const T& yaw, const T& pitch, const T& roll) noexcept
		{
			Matrix4_T<T> const rotX(rotation_x(pitch));
			Matrix4_T<T> const rotY(rotation_y(yaw));
			Matrix4_T<T> const rotZ(rotation_z(roll));
			return rotZ * rotX * rotY;
		}

        template float4x4 scaling(const float& sx, const float& sy, const float& sz) noexcept;
		template <typename T>
		Matrix4_T<T> scaling(const T& sx, const T& sy, const T& sz) noexcept
		{
			return Matrix4_T<T>(
				sx,	0,	0,	0,
				0,	sy,	0,	0,
				0,	0,	sz,	0,
				0,	0,	0,	1);
		}

		template float4x4 scaling(const float3& s) noexcept;
		template <typename T>
		Matrix4_T<T> scaling(const Vector_T<T, 3>& s) noexcept
		{
			return scaling(s.x(), s.y(), s.z());
		}

        template float4x4 mul(const float4x4& lhs, const float4x4& rhs);
        template<typename T>
        Matrix4_T<T> mul(const Matrix4_T<T> &lhs, const Matrix4_T<T> &rhs)
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

        template float4x4 transpose(const float4x4& mat);
        template<typename T>
        Matrix4_T<T> transpose(const Matrix4_T<T> &mat)
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
        template float4x4 inverse(const float4x4& mat);
        template<typename T>
        Matrix4_T<T> inverse(const Matrix4_T<T>& mat)
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
            if (equal<float>(det, 0))
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
        template float4x4 OrthoLH(float w, float h, float nearPlane, float farPlane);
        template<typename T>
        Matrix4_T<T> OrthoLH(T w, T h, T nearPlane, T farPlane)
        {
            const T w_2(w / 2); // w = right - left ，left = -right
            const T h_2(h / 2); // h = top - bottom ，top = -bottom
            return OrthoOffCenterLH(-w_2, w_2, -h_2, h_2, farPlane, nearPlane);            
        }

        // dx->[-1,1][-1,1][0,1]，selected
        // openGL->[-1,1][-1,1][-1,1]
        template float4x4 OrthoOffCenterLH(float left, float right, float bottom, float top, float farPlane, float nearPlane);
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
        template float4x4 LookAtRH(const float3& Eye, const float3& At, const float3& Up);
        template<typename T>
        Matrix4_T<T> LookAtRH(const Vector_T<T, 3>& Eye, const Vector_T<T, 3>& At, const Vector_T<T, 3>& Up)
        {
            Vector_T<T, 3> ZAxis = normalize(Eye - At);// 朝-z方向
            Vector_T<T, 3> XAxis = normalize(Up ^ ZAxis);
            Vector_T<T, 3> YAxis = ZAxis ^ XAxis;
            return Matrix4_T<T>(
                XAxis.x(),      YAxis.x(),      ZAxis.x(),      0,
                XAxis.y(),      YAxis.y(),      ZAxis.y(),      0,
                XAxis.z(),      YAxis.z(),      ZAxis.z(),      0,
                -XAxis | Eye,   -YAxis | Eye,   -ZAxis | Eye,   1);        
        }

        // https://learn.microsoft.com/en-us/previous-versions/windows/desktop/bb281710(v=vs.85)
        // Eye 是摄像机位置，At 是摄像机朝向方向，Up 摄像机向上方向
        template float4x4 LookAtLH(const float3& Eye, const float3& At, const float3& Up);
        template<typename T>
        Matrix4_T<T> LookAtLH(const Vector_T<T, 3>& Eye, const Vector_T<T, 3>& At, const Vector_T<T, 3>& Up)
        {
            Vector_T<T, 3> ZAxis = normalize(At - Eye);
            Vector_T<T, 3> XAxis = normalize(Up ^ ZAxis);
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

        template void decompose(float3& scale, quater& rot, float3& trans, const float4x4& m);
        template<typename T>
        void decompose(Vector_T<T, 3>& scale, Quaternion_T<T>& rot, Vector_T<T, 3>& trans, const Matrix4_T<T>& m)
        {
            // S=> M去掉T得m3x3矩阵，（RS）^T*RT=M3^T*M3=S*S
            scale.x() = sqrt(m(0, 0) * m(0, 0) + m(0, 1) * m(0, 1) + m(0, 2) * m(0, 2));
            scale.y() = sqrt(m(1, 0) * m(1, 0) + m(1, 1) * m(1, 1) + m(1, 2) * m(1, 2));
            scale.z() = sqrt(m(2, 0) * m(2, 0) + m(2, 1) * m(2, 1) + m(2, 2) * m(2, 2));

            // T
            trans = Vector_T<T, 3>(m(3, 0), m(3, 1), m(3, 2));

            // R=RS*1/S
            Matrix4_T<T> rot_mat;
            rot_mat(0, 0) = m(0, 0) / scale.x();
            rot_mat(0, 1) = m(0, 1) / scale.x();
            rot_mat(0, 2) = m(0, 2) / scale.x();
            rot_mat(0, 3) = 0;
            rot_mat(1, 0) = m(1, 0) / scale.y();
            rot_mat(1, 1) = m(1, 1) / scale.y();
            rot_mat(1, 2) = m(1, 2) / scale.y();
            rot_mat(1, 3) = 0;
            rot_mat(2, 0) = m(2, 0) / scale.z();
            rot_mat(2, 1) = m(2, 1) / scale.z();
            rot_mat(2, 2) = m(2, 2) / scale.z();
            rot_mat(2, 3) = 0;
            rot_mat(3, 0) = 0;
            rot_mat(3, 1) = 0;
            rot_mat(3, 2) = 0;
            rot_mat(3, 3) = 1;
            rot = to_quaternion(rot_mat);
        }

        template float4x4 transformation(const float3* scaling_center, const quater* scaling_rotation, const float3* scale,
			const float3* rotation_center, const quater* rotation, const float3* trans) noexcept;
		template <typename T>
		Matrix4_T<T> transformation(const Vector_T<T, 3>* scaling_center, const Quaternion_T<T>* scaling_rotation, const Vector_T<T, 3>* scale,
			const Vector_T<T, 3>* rotation_center, const Quaternion_T<T>* rotation, const Vector_T<T, 3>* trans) noexcept
		{
			Vector_T<T, 3> psc, prc, pt;
			if (scaling_center)
			{
				psc = *scaling_center;
			}
			else
			{
				psc = Vector_T<T, 3>(T(0), T(0), T(0));
			}
			if (rotation_center)
			{
				prc = *rotation_center;
			}
			else
			{
				prc = Vector_T<T, 3>(T(0), T(0), T(0));
			}
			if (trans)
			{
				pt = *trans;
			}
			else
			{
				pt = Vector_T<T, 3>(T(0), T(0), T(0));
			}

			Matrix4_T<T> m1, m2, m3, m4, m5, m6, m7;
			m1 = translation(-psc);
			if (scaling_rotation)
			{
				m4 = to_matrix(*scaling_rotation);
				m2 = inverse(m4);
			}
			else
			{
				m2 = m4 = Matrix4_T<T>::Identity();
			}
			if (scale)
			{
				m3 = scaling(*scale);
			}
			else
			{
				m3 = Matrix4_T<T>::Identity();
			}
			if (rotation)
			{
				m6 = to_matrix(*rotation);
			}
			else
			{
				m6 = Matrix4_T<T>::Identity();
			}
			m5 = translation(psc - prc);
			m7 = translation(prc + pt);

			return m1 * m2 * m3 * m4 * m5 * m6 * m7;
		}

        template quater mul(const quater& lhs, const quater& rhs) noexcept;
        template<typename T>
        Quaternion_T<T> mul(const Quaternion_T<T>& lhs, const Quaternion_T<T>& rhs) noexcept
        {
            return Quaternion_T<T>(
                lhs.x() * rhs.w() - lhs.y() * rhs.z() + lhs.z() * rhs.y() + lhs.w() * rhs.x(),
                lhs.x() * rhs.z() + lhs.y() * rhs.w() - lhs.z() * rhs.x() + lhs.w() * rhs.y(),
                lhs.y() * rhs.x() - lhs.x() * rhs.y() + lhs.z() * rhs.w() + lhs.w() * rhs.z(),
                lhs.w() * rhs.w() - lhs.x() * rhs.x() - lhs.y() * rhs.y() - lhs.z() * rhs.z());
        }

        template quater conjugate(const  quater& rhs) noexcept;
        template <typename T>
        Quaternion_T<T> conjugate(const Quaternion_T<T>&rhs) noexcept
        {
            return Quaternion_T<T>(-rhs.x(), -rhs.y(), -rhs.z(), rhs.w());
        }

        template std::pair<quater, quater> conjugate(const quater& real, const quater& dual) noexcept;
		template <typename T>
		std::pair<Quaternion_T<T>, Quaternion_T<T>> conjugate(const Quaternion_T<T>&real, const Quaternion_T<T>& dual) noexcept
		{
			return std::make_pair(conjugate(real), conjugate(dual));
		}

        template quater inverse(const quater& rhs) noexcept;
        template <typename T>
        Quaternion_T<T> inverse(const Quaternion_T<T>& rhs) noexcept
        {
            T var(T(1) / length(rhs));
            return Quaternion_T<T>(-rhs.x() * var, -rhs.y() * var, -rhs.z() * var, rhs.w() * var);
        }

		template std::pair<quater, quater> inverse(const  quater& real, const  quater& dual) noexcept;
        template <typename T>
		std::pair<Quaternion_T<T>, Quaternion_T<T>> inverse(const Quaternion_T<T>& real, const Quaternion_T<T>& dual) noexcept
		{
			float const sqr_len_0 = dot(real, real);
			float const sqr_len_e = 2.0f * dot(real, dual);
			float const inv_sqr_len_0 = 1.0f / sqr_len_0;
			float const inv_sqr_len_e = -sqr_len_e / (sqr_len_0 * sqr_len_0);
			std::pair<Quaternion_T<T>, Quaternion_T<T>> conj = conjugate(real, dual);
			return std::make_pair(inv_sqr_len_0 * conj.first, inv_sqr_len_0 * conj.second + inv_sqr_len_e * conj.first);
		}

        template quater slerp(quater const & lhs, quater const & rhs, float s) noexcept;
		template <typename T>
		Quaternion_T<T> slerp(Quaternion_T<T> const & lhs, Quaternion_T<T> const & rhs, T s) noexcept
		{
			T scale0, scale1;

			// DOT the quats to get the cosine of the angle between them
			T cosom = dot(lhs, rhs);

			T dir = T(1);
			if (cosom < 0)
			{
				dir = T(-1);
				cosom = -cosom;
			}

			// make sure they are different enough to avoid a divide by 0
			if (cosom < T(1) - std::numeric_limits<T>::epsilon())
			{
				// SLERP away
				T const omega = acos(cosom);
				T const isinom = T(1) / sin(omega);
				scale0 = sin((T(1) - s) * omega) * isinom;
				scale1 = sin(s * omega) * isinom;
			}
			else
			{
				// LERP is good enough at this distance
				scale0 = T(1) - s;
				scale1 = s;
			}

			// Compute the result
			return scale0 * lhs + dir * scale1 * rhs;
		}

        template float4x4 to_matrix(const quater &quat);
        template<typename T>
        Matrix4_T<T> to_matrix(const Quaternion_T<T>& quat)
        {
            // calculate coefficients
            const T x2(quat.x() + quat.x());
            const T y2(quat.y() + quat.y());
            const T z2(quat.z() + quat.z());

            const T xx2(quat.x() * x2), xy2(quat.x() * y2), xz2(quat.x() * z2);
            const T yy2(quat.y() * y2), yz2(quat.y() * z2), zz2(quat.z() * z2);
            const T wx2(quat.w() * x2), wy2(quat.w() * y2), wz2(quat.w() * z2);

            return Matrix4_T<T>(
                1 - yy2 - zz2,	xy2 + wz2,		xz2 - wy2,		0,
                xy2 - wz2,		1 - xx2 - zz2,	yz2 + wx2,		0,
                xz2 + wy2,		yz2 - wx2,		1 - xx2 - yy2,	0,
                0,				0,				0,				1);
        }

        template float4x4 to_matrix(const rotator &rot);
        template<typename T>
        Matrix4_T<T> to_matrix(const Rotator_T<T>& rot)
        {
            Matrix4_T<T> rot_x = rotation_x(rot.pitch());
            Matrix4_T<T> rot_y = rotation_y(rot.yaw());
            Matrix4_T<T> rot_z = rotation_z(rot.roll());
            return rot_x * rot_y * rot_z;
        }

        template quater to_quaternion(const float4x4 &mat);
        template<typename T>
        Quaternion_T<T> to_quaternion(const Matrix4_T<T>& mat)
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

                    if (!equal(s, 0.f))
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

                    if (!equal(s, 0.f))
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

                    if (!equal(s, 0.f))
                    {
                        s = 0.5f / s;
                    }

                    quat.w() = (mat(0, 1) - mat(1, 0)) * s;
                    quat.x() = (mat(0, 2) + mat(2, 0)) * s;
                    quat.y() = (mat(1, 2) + mat(2, 1)) * s;
                    break;
                }
            }

            return normalize(quat);
        }

        template quater to_quaternion(float3 const & tangent, float3 const & binormal, float3 const & normal, uint32_t bits) noexcept;
		template <typename T>
		Quaternion_T<T> to_quaternion(Vector_T<T, 3> const & tangent, Vector_T<T, 3> const & binormal, Vector_T<T, 3> const & normal, uint32_t bits) noexcept
		{
			T k = 1;
			if (dot(binormal, cross(normal, tangent)) < 0)
			{
				k = -1;
			}

			Matrix4_T<T> tangent_frame(tangent.x(), tangent.y(), tangent.z(), 0,
				k * binormal.x(), k * binormal.y(), k * binormal.z(), 0,
				normal.x(), normal.y(), normal.z(), 0,
				0, 0, 0, 1);
			Quaternion_T<T> tangent_quat = to_quaternion(tangent_frame);
			if (tangent_quat.w() < 0)
			{
				tangent_quat = -tangent_quat;
			}
			if (bits > 0)
			{
				T const bias = T(1) / ((1UL << (bits - 1)) - 1);
				if (tangent_quat.w() < bias)
				{
					T const factor = sqrt(1 - bias * bias);
					tangent_quat.x() *= factor;
					tangent_quat.y() *= factor;
					tangent_quat.z() *= factor;
					tangent_quat.w() = bias;
				}
			}
			if (k < 0)
			{
				tangent_quat = -tangent_quat;
			}

			return tangent_quat;
		}

        template quater ToQuaternion(const rotator &rot);
        template<typename T>
        Quaternion_T<T> ToQuaternion(const Rotator_T<T>& rot)
        {
            const T angX(rot.pitch() / 2), angY(rot.yaw() / 2), angZ(rot.roll() / 2);
            T sx, sy, sz;
            T cx, cy, cz;
            sincos(angX, sx, cx);
            sincos(angY, sy, cy);
            sincos(angZ, sz, cz);

            return quater(
                sx * cy * cz + cx * sy * sz,
                cx * sy * cz - sx * cy * sz,
                cx * cy * sz - sx * sy * cz,
                sx * sy * sz + cx * cy * cz);
        }

        //template rotator ToRotator(const float4x4 &mat);
        //template<typename T>
        //Rotator_T<float> ToRotator(const Matrix4_T<T>& mat)
        //{
        //     return rotator();
        //}

        // From http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/index.htm
        template rotator ToRotator(const quater &quat);
        template<typename T>
        Rotator_T<T> ToRotator(const Quaternion_T<T>& quat)
        {
            T sqx = quat.x() * quat.x();
            T sqy = quat.y() * quat.y();
            T sqz = quat.z() * quat.z();
            T sqw = quat.w() * quat.w();
            T unit = sqx + sqy + sqz + sqw;
            T test = quat.w() * quat.x() + quat.y() * quat.z();
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

        template void ToYawPitchRoll(float& yaw, float& pitch, float& roll, const quater& quat);
        template<typename T>
        void ToYawPitchRoll(T& yaw, T& pitch, T& roll, const Quaternion_T<T>& quat)
        {
            T sqx = quat.x() * quat.x();
            T sqy = quat.y() * quat.y();
            T sqz = quat.z() * quat.z();
            T sqw = quat.w() * quat.w();
            T unit = sqx + sqy + sqz + sqw;
            T test = quat.w() * quat.x() + quat.y() * quat.z();
            rotator rot;
            if (test > 0.499f * unit)
            {
                // singularity at north pole
                yaw = 2 * atan2(quat.z(), quat.w());
                pitch = PI / 2;
                roll = 0;
            }
            else
            {
                if (test < -(0.499f) * unit)
                {
                    // singularity at south pole
                    yaw = -2 * atan2(quat.z(), quat.w());
                    pitch = -PI / 2;
                    roll = 0;
                }
                else
                {
                    yaw = atan2(2 * (quat.y() * quat.w() - quat.x() * quat.z()), -sqx - sqy + sqz + sqw);
                    pitch = asin(2 * test / unit);
                    roll = atan2(2 * (quat.z() * quat.w() - quat.x() * quat.y()), -sqx + sqy - sqz + sqw);
                }
            }
        }

		template quater mul_real(const  quater& lhs_real, const  quater& rhs_real) noexcept;
		template <typename T>
		Quaternion_T<T> mul_real(const  Quaternion_T<T>& lhs_real, const  Quaternion_T<T>& rhs_real) noexcept
		{
			return lhs_real * rhs_real;
		}

		template quater mul_dual(const  quater& lhs_real, const  quater& lhs_dual,
			const  quater& rhs_real, const  quater& rhs_dual) noexcept;
		template <typename T>
		Quaternion_T<T> mul_dual(const  Quaternion_T<T>& lhs_real, const  Quaternion_T<T>& lhs_dual,
			const  Quaternion_T<T>& rhs_real, const  Quaternion_T<T>& rhs_dual) noexcept
		{
			return lhs_real * rhs_dual + lhs_dual * rhs_real;
		}

        template quater quat_trans_to_udq(const  quater& q, float3 const & t) noexcept;
		template <typename T>
		Quaternion_T<T> quat_trans_to_udq(const Quaternion_T<T>&q, Vector_T<T, 3> const & t) noexcept
		{
			return mul(q, Quaternion_T<T>(T(0.5) * t.x(), T(0.5) * t.y(), T(0.5) * t.z(), T(0.0)));
		}

        template float4x4 udq_to_matrix(const  quater& real, const  quater& dual) noexcept;
		template <typename T>
		Matrix4_T<T> udq_to_matrix(const Quaternion_T<T>&real, const Quaternion_T<T>&dual) noexcept
		{
			Matrix4_T<T> m;

			float len2 = dot(real, real);
			float w = real.w(), x = real.x(), y = real.y(), z = real.z();
			float t0 = dual.w(), t1 = dual.x(), t2 = dual.y(), t3 = dual.z();

			m(0, 0) = w * w + x * x - y * y - z * z;
			m(1, 0) = 2 * x * y - 2 * w * z;
			m(2, 0) = 2 * x * z + 2 * w * y;
			m(0, 1) = 2 * x * y + 2 * w * z;
			m(1, 1) = w * w + y * y - x * x - z * z;
			m(2, 1) = 2 * y * z - 2 * w * x;
			m(0, 2) = 2 * x * z - 2 * w * y;
			m(1, 2) = 2 * y * z + 2 * w * x;
			m(2, 2) = w * w + z * z - x * x - y * y;

			m(3, 0) = -2 * t0 * x + 2 * w * t1 - 2 * t2 * z + 2 * y * t3;
			m(3, 1) = -2 * t0 * y + 2 * t1 * z - 2 * x * t3 + 2 * w * t2;
			m(3, 2) = -2 * t0 * z + 2 * x * t2 + 2 * w * t3 - 2 * t1 * y;

			m(0, 3) = 0;
			m(1, 3) = 0;
			m(2, 3) = 0;
			m(3, 3) = len2;

			m /= len2;

			return m;
		}

        template float3 udq_to_trans(const  quater& real, const  quater& dual) noexcept;
		template <typename T>
		Vector_T<T, 3> udq_to_trans(const  Quaternion_T<T>& real, const  Quaternion_T<T>& dual) noexcept
		{
			Quaternion_T<T> qeq0 = mul(conjugate(real), dual);
			return T(2.0) * Vector_T<T, 3>(qeq0.x(), qeq0.y(), qeq0.z());
		}

        
		template void udq_to_screw(float& angle, float& pitch, float3& dir, float3& moment,
			quater const & real, quater const & dual) noexcept;
		template <typename T>
		void udq_to_screw(T& angle, T& pitch, Vector_T<T, 3>& dir, Vector_T<T, 3>& moment,
			Quaternion_T<T> const & real, Quaternion_T<T> const & dual) noexcept
		{
			if (abs(real.w()) >= 1)
			{
				// pure translation

				angle = 0;
				dir = dual.v();

				T dir_sq_len = length_sq(dir);

				if (dir_sq_len > T(1e-6))
				{
					T dir_len = sqrt(dir_sq_len);
					pitch = 2 * dir_len;
					dir /= dir_len;
				}
				else
				{
					pitch = 0;
				}

				moment = Vector_T<T, 3>::Zero();
			}
			else
			{ 
				angle = 2 * acos(real.w());

				float const s = length_sq(real.v());
				if (s < T(1e-6))
				{
					dir = Vector_T<T, 3>::Zero();
					pitch = 0;
					moment = Vector_T<T, 3>::Zero();
				}
				else
				{
					float oos = recip_sqrt(s);
					dir = real.v() * oos;

					pitch = -2 * dual.w() * oos;

					moment = (dual.v() - dir * pitch * real.w() * T(0.5)) * oos;
				}
			}
		}

        template std::pair<quater, quater> udq_from_screw(float const & angle, float const & pitch, float3 const & dir, float3 const & moment) noexcept;
		template <typename T>
		std::pair<Quaternion_T<T>, Quaternion_T<T>> udq_from_screw(T const & angle, T const & pitch, Vector_T<T, 3> const & dir, Vector_T<T, 3> const & moment) noexcept
		{
			T sa, ca;
			sincos(angle * T(0.5), sa, ca);
			return std::make_pair(Quaternion_T<T>(dir * sa, ca),
				Quaternion_T<T>(sa * moment + T(0.5) * pitch * ca * dir, -pitch * sa * T(0.5)));
		}

        template std::pair<quater, quater> sclerp(quater const & lhs_real, quater const & lhs_dual,
			quater const & rhs_real, quater const & rhs_dual, float s) noexcept;
		template <typename T>
		std::pair<Quaternion_T<T>, Quaternion_T<T>> sclerp(Quaternion_T<T> const & lhs_real, Quaternion_T<T> const & lhs_dual,
			Quaternion_T<T> const & rhs_real, Quaternion_T<T> const & rhs_dual, T s) noexcept
		{
			// Make sure dot product is >= 0
			float const quat_dot = dot(lhs_real, rhs_real);
			Quaternion_T<T> to_sign_corrected_real = rhs_real;
			Quaternion_T<T> to_sign_corrected_dual = rhs_dual;
			if (quat_dot < 0)
			{
				to_sign_corrected_real = -to_sign_corrected_real;
				to_sign_corrected_dual = -to_sign_corrected_dual;
			}

			std::pair<Quaternion_T<T>, Quaternion_T<T>> dif_dq = inverse(lhs_real, lhs_dual);
			dif_dq.second = mul_dual(dif_dq.first, dif_dq.second, to_sign_corrected_real, to_sign_corrected_dual);
			dif_dq.first = mul_real(dif_dq.first, to_sign_corrected_real);
	
			float angle, pitch;
			float3 direction, moment;
			udq_to_screw(angle, pitch, direction, moment, dif_dq.first, dif_dq.second);

			angle *= s; 
			pitch *= s;
			dif_dq = udq_from_screw(angle, pitch, direction, moment);

			dif_dq.second = mul_dual(lhs_real, lhs_dual, dif_dq.first, dif_dq.second);
			dif_dq.first = mul_real(lhs_real, dif_dq.first);

			return dif_dq;
		}

		template AABBox compute_aabbox(float3* first, float3* last) noexcept;
		template AABBox compute_aabbox(float4* first, float4* last) noexcept;
		template AABBox compute_aabbox(float3 const * first, float3 const * last) noexcept;
		template AABBox compute_aabbox(float4 const * first, float4 const * last) noexcept;
		template AABBox compute_aabbox(std::vector<float3>::iterator first, std::vector<float3>::iterator last) noexcept;
		template AABBox compute_aabbox(std::vector<float4>::iterator first, std::vector<float4>::iterator last) noexcept;
		template AABBox compute_aabbox(std::vector<float3>::const_iterator first, std::vector<float3>::const_iterator last) noexcept;
		template AABBox compute_aabbox(std::vector<float4>::const_iterator first, std::vector<float4>::const_iterator last) noexcept;
		template <typename Iterator>
		AABBox_T<typename std::iterator_traits<Iterator>::value_type::value_type> compute_aabbox(Iterator first, Iterator last) noexcept
		{
			typedef typename std::iterator_traits<Iterator>::value_type::value_type value_type;

			Vector_T<value_type, 3> minVec = *first;
			Vector_T<value_type, 3> maxVec = *first;
			Iterator iter = first;
			++ iter;
			for (; iter != last; ++ iter)
			{
				Vector_T<value_type, 3> const & v = *iter;
				minVec = minimize(minVec, v);
				maxVec = maximize(maxVec, v);
			}
			return AABBox_T<value_type>(minVec, maxVec);
		}

		template AABBox transform_aabb(AABBox const & aabb, float4x4 const & mat) noexcept;
		template <typename T>
		AABBox_T<T> transform_aabb(AABBox_T<T> const & aabb, Matrix4_T<T> const & mat) noexcept
		{
			Vector_T<T, 3> min, max;
			min = max = transform_coord(aabb.Corner(0), mat);
			for (size_t j = 1; j < 8; ++j)
			{
				Vector_T<T, 3> const vec = transform_coord(aabb.Corner(j), mat);
				min = minimize(min, vec);
				max = maximize(max, vec);
			}

			return AABBox_T<T>(min, max);
		}

		template AABBox transform_aabb(AABBox const & aabb, float3 const & scale, quater const & rot, float3 const & trans) noexcept;
		template <typename T>
		AABBox_T<T> transform_aabb(AABBox_T<T> const & aabb, Vector_T<T, 3> const & scale, Quaternion_T<T> const & rot, Vector_T<T, 3> const & trans) noexcept
		{
			Vector_T<T, 3> min, max;
			min = max = transform_quat(aabb.Corner(0) * scale, rot) + trans;
			for (size_t j = 1; j < 8; ++ j)
			{
				Vector_T<T, 3> const vec = transform_quat(aabb.Corner(j) * scale, rot) + trans;
				min = minimize(min, vec);
				max = maximize(max, vec);
			}

			return AABBox_T<T>(min, max);
		}

    }
}