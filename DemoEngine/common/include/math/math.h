#pragma once
#include <stdint.h>
#include <cmath>
#include <algorithm>

namespace RenderWorker
{
	namespace MathWorker
	{
		template<typename T, size_t SIZE>
		class Vector_T;
		template <typename T>
		class Quaternion_T;
		template <typename T>
		class Matrix4_T;
		template <typename T>
		class Rotator_T;
		template <typename T>
		class Color_T;

		// 常量定义
		/////////////////////////////////////////////////////////////////////////////////
		float constexpr PI		= 3.141592f;			// PI
		float constexpr PI2		= 6.283185f;			// PI * 2
		float constexpr PIdiv2	= 1.570796f;			// PI / 2

		float constexpr DEG90	= 1.570796f;			// 90 度
		float constexpr DEG270	= -1.570796f;			// 270 度
		float constexpr DEG45	= 0.7853981f;			// 45 度
		float constexpr DEG5	= 0.0872664f;			// 5 度
		float constexpr DEG10	= 0.1745329f;			// 10 度
		float constexpr DEG20	= 0.3490658f;			// 20 度
		float constexpr DEG30	= 0.5235987f;			// 30 度
		float constexpr DEG60	= 1.047197f;			// 60 度
		float constexpr DEG120	= 2.094395f;			// 120 度

		float constexpr DEG40	= 0.6981317f;			// 40 度
		float constexpr DEG80	= 1.396263f;			// 80 度
		float constexpr DEG140	= 2.443460f;			// 140 度
		float constexpr DEG160	= 2.792526f;			// 160 度

		float constexpr SQRT2	= 1.414213f;			// 根2
		float constexpr SQRT_2	= 0.7071068f;			// 1 / SQRT2
		float constexpr SQRT3	= 1.732050f;			// 根3

		float constexpr DEG2RAD	= 0.01745329f;			// 角度化弧度因数
		float constexpr RAD2DEG	= 57.29577f;			// 弧度化角度因数

		inline float Deg2Rad(const float x) { return x * DEG2RAD; }
		inline float Rad2Deg(const float x) {return x * RAD2DEG;}
		
		float sin(float x) noexcept;
		float cos(float x) noexcept;
		void SinCos(float fAnglel, float& s, float& c) noexcept;

		// 求绝对值
		template <typename T>
		inline T
		abs(const T & x) noexcept
		{
			return x < T(0) ? -x : x;
		}		
		template <typename T, size_t N>
		Vector_T<T, N> abs(Vector_T<T, N> const & x) noexcept;

		// 判断两个数是否相等
		template <typename T>
		inline bool
		equal(T const & lhs, T const & rhs) noexcept
		{
			return (lhs == rhs);
		}
		// 浮点版本
		template <>
		inline bool
		equal<float>(float const & lhs, float const & rhs) noexcept
		{
			return (abs<float>(lhs - rhs)
				<= std::numeric_limits<float>::epsilon());
		}
		template <>
		inline bool
		equal<double>(double const & lhs, double const & rhs) noexcept
		{
			return (abs<double>(lhs - rhs)
				<= std::numeric_limits<double>::epsilon());
		}

		//平方根倒数速算法
		float RecipSqrt(float number) noexcept;
		
		//限制 val 在 low 和 high 之间 
		template <typename T>
		inline T const &
		clamp(T const & val, T const & low, T const & high) noexcept
		{
			return std::max(low, std::min(high, val));
		}

		//叉积    
		template<typename T>
		T cross(const Vector_T<T, 2> & lhs, const Vector_T<T, 2> & rhs) noexcept;
		template<typename T>
		Vector_T<T, 3> cross(const Vector_T<T, 3> & lhs, const Vector_T<T, 3> & rhs) noexcept;
		template<typename T>
		Vector_T<T, 4> cross(const Vector_T<T, 4> & lhs, const Vector_T<T, 4> & rhs) noexcept;

		//点积    
		template<typename T>
		typename T::value_type dot(const T & lhs, const T & rhs) noexcept;

		//长度的平方
		template<typename T>
		typename T::value_type length_sq(const T & rhs) noexcept;

		//求模，向量的长
		template<typename T>
		typename T::value_type length(const T & rhs) noexcept;

		//线性插值
		template <typename T>
		T lerp(const T& lhs, const T& rhs, float s) noexcept;

		//向量标准化
		template<typename T>
		T normalize(const T & rhs) noexcept;

		// 最大值
		template <typename T>
		T maximize(const T & lhs, const T & rhs) noexcept;

		// 最小值
		template <typename T>
		T minimize(const T & lhs, const T & rhs) noexcept;

		// 向量与矩阵的乘法
		template <typename T>
		Vector_T<typename T::value_type, 4> transform(T const & v, Matrix4_T<typename T::value_type> const & mat) noexcept;
		
		//返回 from 与 to 之间的角度
		template<typename T>
		typename T::value_type Angle(const T& from, const T& to);
		



		// Color space
		///////////////////////////////////////////////////////////////////////////////
		float linear_to_srgb(float linear) noexcept;
		float srgb_to_linear(float srgb) noexcept;
		
		//四元数乘法
		template <typename T>
		Quaternion_T<T> mul(Quaternion_T<T> const & lhs, Quaternion_T<T> const & rhs) noexcept;

		//四元数共轭
		template <typename T>
		Quaternion_T<T> Conjugate(Quaternion_T<T> const & rhs) noexcept;

		//四元数的逆
		template <typename T>
		Quaternion_T<T> Inverse(const Quaternion_T<T>& rhs) noexcept;



		//矩形平移
		template<typename T>
		Matrix4_T<T> Translation(T X, T Y, T Z);
		template<typename T>
		Matrix4_T<T> Translation(const Vector_T<T, 3>& Move);

		// 矩形缩放
		template<typename T>
		Matrix4_T<T> MatrixScale(T X, T Y, T Z);
		template<typename T>
		Matrix4_T<T> MatrixScale(const Vector_T<T, 3>& Scale);

		//矩阵旋转
		template<typename T>
		Matrix4_T<T> MatrixRotateX(T Angle);
		template<typename T>
		Matrix4_T<T> MatrixRotateY(T Angle);
		template<typename T>
		Matrix4_T<T> MatrixRotateZ(T Angle);
		template<typename T>
		Matrix4_T<T> MatrixRotate(const Vector_T<T, 3>& Pos, T Angle);

		//矩形乘法
		template<typename T>
		Matrix4_T<T> mul(const Matrix4_T<T>&lhs, const Matrix4_T<T>& rhs);

		//矩阵转置
		template<typename T>
		Matrix4_T<T> Transpose(const Matrix4_T<T>& m);

		//矩阵的行列式
		template<typename T>
		T Determinant(const Matrix4_T<T>& m);

		//矩阵的逆
		template<typename T>
		Matrix4_T<T> Inverse(const Matrix4_T<T>& m);

		template <typename T>
		Matrix4_T<T> LHToRH(Matrix4_T<T> const & rhs) noexcept;

		template<typename T>
		Matrix4_T<T> LookAtRH(const Vector_T<T, 3>& Eye, const Vector_T<T, 3>& At, const Vector_T<T, 3>& Up);
		template<typename T>
		Matrix4_T<T> LookAtLH(const Vector_T<T, 3>& Eye, const Vector_T<T, 3>& At, const Vector_T<T, 3>& Up);

		// 正交投影
		//left hand , z axis = zero.->directX
		template<typename T>
		Matrix4_T<T> OrthoLH(T w, T h, T farPlane, T nearPlane);
		template<typename T>
		Matrix4_T<T> OrthoOffCenterLH(T left, T right, T bottom, T top, T farPlane, T nearPlane);
		
		// right hand , z axis = negative one ->openGL
		template<typename T>
		Matrix4_T<T> OrthoRH(T w, T h, T farPlane, T nearPlane);
		template<typename T>
		Matrix4_T<T> OrthoOffCenterRH(T left, T right, T bottom, T top, T farPlane, T nearPlane);
		
		//透视投影
		//left hand , z axis = zero.->directX
		template<typename T>
		Matrix4_T<T> PerspectiveLH(T w, T h, T Near, T Far);
		template<typename T>
		Matrix4_T<T> PerspectiveOffCenterLH(T left, T right, T bottom, T top, T farPlane, T nearPlane);
		template<typename T>
		Matrix4_T<T> PerspectiveFovLH(T Fov, T Aspect, T Near, T Far);

		// right hand , z axis = negative one ->openGL
		template<typename T>
		Matrix4_T<T> PerspectiveRH(T w, T h, T Near, T Far);
		template<typename T>
		Matrix4_T<T> PerspectiveOffCenterRH(T left, T right, T bottom, T top, T farPlane, T nearPlane);
		template<typename T>
		Matrix4_T<T> PerspectiveFovRH(T Fov, T Aspect, T Near, T Far);

		// 矩阵分解
		template<typename T>
		void Decompose(Vector_T<T, 3>& scale, Quaternion_T<T>& rot, Vector_T<T, 3>& trans, const Matrix4_T<T>& m);
		
		//相互转换
		template<typename T>
		Matrix4_T<T> ToMatrix(const Quaternion_T<T>& quat);
		template<typename T>
		Matrix4_T<T> ToMatrix(const Rotator_T<T>& rot);

		template<typename T>
		Quaternion_T<T> ToQuaternion(const Matrix4_T<T>& mat);
		template<typename T>
		Quaternion_T<T> ToQuaternion(const Rotator_T<T>& rot);

		//template<typename T>
		//Rotator_T<T> ToRotator(const Matrix4_T<T>& mat);
		template<typename T>
		Rotator_T<T> ToRotator(const Quaternion_T<T>& quat);
		template<typename T>
		void ToYawPitchRoll(T& yaw, T& pitch, T& roll, const Quaternion_T<T>& quat);

		template<typename T>
		Vector_T<T, 3> TransformQuat(const Vector_T<T, 3>& v, const Quaternion_T<T>& quat);
	}
}

#include <math/vectorxd.h>
#include <math/matrix.h>
#include <math/quaternion.h>
#include <math/rotator.h>
#include <math/color.h>
#include <math/half.h>
