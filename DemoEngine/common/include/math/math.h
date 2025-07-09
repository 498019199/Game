#pragma once
#include <stdint.h>
#include <cmath>
#include <algorithm>

namespace MathWorker
{
	template<typename T, int SIZE>
	class Vector_T;
	using int1 = Vector_T<int32_t, 1>;
	using int2 = Vector_T<int32_t, 2>;
	using int3 = Vector_T<int32_t, 3>;
	using int4 = Vector_T<int32_t, 4>;
	using uint1 = Vector_T<uint32_t, 1>;
	using uint2 = Vector_T<uint32_t, 2>;
	using uint3 = Vector_T<uint32_t, 3>;
	using uint4 = Vector_T<uint32_t, 4>;
	using float1 = Vector_T<float, 1>;
	using float2 = Vector_T<float, 2>;
	using float3 = Vector_T<float, 3>;
	using float4 = Vector_T<float, 4>;

	template <typename T>
	class Quaternion_T;
	using quater = Quaternion_T<float>;

	template <typename T>
	class Matrix4_T;
	using float4x4 = Matrix4_T<float>;

    template <typename T>
	class Rotator_T;
    using rotator = Rotator_T<float>;

	template <typename T>
	class Color_T;
    using Color = Color_T<float>;
}

namespace MathWorker
{
    const float PI = 3.141592f;
	const float PI2 = 6.283185f;
	const float PIdiv2 = 1.570796f;

	const float DEG90 = 1.570796f;			// 90 度   
	const float DEG270 = -1.570796f;		// 270 度   
	const float DEG45 = 0.7853981f;			// 45 度  
	const float DEG5 = 0.0872664f;			// 5 度  
	const float DEG10 = 0.1745329f;			// 10 度  
	const float DEG20 = 0.3490658f;			// 20 度  
	const float DEG30 = 0.5235987f;			// 30 度  
	const float DEG60 = 1.047197f;			// 60  度 
	const float DEG120 = 2.094395f;			// 120 度  
	const float DEG40 = 0.6981317f;			// 40  度 
	const float DEG80 = 1.396263f;			// 80 度   
	const float DEG140 = 2.443460f;			// 140 度  
	const float DEG160 = 2.792526f;			// 160 度  

	const float DEG2RAD = 0.01745329f;			//角度化弧度因数
	const float RAD2DEG = 57.29577f;			//弧度化角度因数

	inline float Deg2Rad(const float x) { return x * DEG2RAD; }
	inline float Rad2Deg(const float x) {return x * RAD2DEG;}
	
	float sin(float x) noexcept;
	float cos(float x) noexcept;
	void SinCos(float fAnglel, float& s, float& c) noexcept;

	//浮点相等        
	template<typename T>
	bool IsEqual(T X, T Y);

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
	T Cross(const Vector_T<T, 2> & lhs, const Vector_T<T, 2> & rhs) noexcept;
	template<typename T>
	Vector_T<T, 3> Cross(const Vector_T<T, 3> & lhs, const Vector_T<T, 3> & rhs) noexcept;
	template<typename T>
	Vector_T<T, 4> Cross(const Vector_T<T, 4> & lhs, const Vector_T<T, 4> & rhs) noexcept;

	//点积    
	template<typename T>
	typename T::value_type Dot(const T & lhs, const T & rhs) noexcept;

	//长度的平方
	template<typename T>
	typename T::value_type LengthSq(const T & rhs) noexcept;

	//求模，向量的长
	template<typename T>
	typename T::value_type Length(const T & rhs) noexcept;

	//向量标准化
	template<typename T>
	T Normalize(const T & rhs) noexcept;

	//距离     
	template<typename T>
	T Distance(const Vector_T<T, 2> & lhs, const Vector_T<T, 2> & rhs) noexcept;
	template<typename T>
	T Distance(const Vector_T<T, 3> & lhs, const Vector_T<T, 3> & rhs) noexcept;
    
    //返回 from 与 to 之间的角度
    template<typename T>
    typename T::value_type Angle(const T& from, const T& to);
	
	//线性插值
	template <typename T>
	T Lerp(const T& lhs, const T& rhs, float s) noexcept;


	// Color space
	///////////////////////////////////////////////////////////////////////////////
	float linear_to_srgb(float linear) noexcept;
	float srgb_to_linear(float srgb) noexcept;
	
	//四元数乘法
	template <typename T>
	Quaternion_T<T> Mul(Quaternion_T<T> const & lhs, Quaternion_T<T> const & rhs) noexcept;

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
	Matrix4_T<T> Mul(const Matrix4_T<T>&lhs, const Matrix4_T<T>& rhs);
	//行向量X矩形
	template<typename T>
	Vector_T<T, 4> Mul(const Vector_T<T, 4>&lhs, const Matrix4_T<T>& rhs);

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


#include <math/vectorxd.h>
#include <math/matrix.h>
#include <math/quaternion.h>
#include <math/rotator.h>
#include <math/color.h>
#include <math/half.h>
