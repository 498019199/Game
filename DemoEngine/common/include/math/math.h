#pragma once
#include <stdint.h>
#include <cmath>
#include <algorithm>

namespace RenderWorker
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
	template <typename T>
	class AABBox_T;

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
	
	enum class BoundOverlap : uint32_t
	{
		No = 0,
		Partial,
		Yes,
	};

	namespace MathWorker
	{
		inline float Deg2Rad(const float x) { return x * DEG2RAD; }
		inline float Rad2Deg(const float x) {return x * RAD2DEG;}
		
		float sin(float x) noexcept;
		float cos(float x) noexcept;
		void sincos(float fAnglel, float& s, float& c) noexcept;

		// 求绝对值
		template <typename T>
		inline T
		abs(const T & x) noexcept
		{
			return x < T(0) ? -x : x;
		}		
		template <typename T, size_t N>
		Vector_T<T, N> abs(Vector_T<T, N> const & x) noexcept;

		// 取符号
		template <typename T>
		inline T
		sgn(T const & x) noexcept
		{
			return x < T(0) ? T(-1) : (x > T(0) ? T(1) : T(0));
		}
		template <typename T, size_t N>
		Vector_T<T, N> sgn(Vector_T<T, N> const & x) noexcept;

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

		float sqrt(float x) noexcept;
		//平方根倒数速算法
		float recip_sqrt(float number) noexcept;
		
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

		template <typename T>
		Vector_T<T, 3> transform_quat(Vector_T<T, 3> const & v, Quaternion_T<T> const & quat) noexcept;

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

		// 取三个中小的
		template <typename T>
		inline T const &
		min3(T const & a, T const & b, T const & c) noexcept
		{
			return std::min(std::min(a, b), c);
		}
		// 取三个中大的
		template <typename T>
		inline T const &
		max3(T const & a, T const & b, T const & c) noexcept
		{
			return std::max(std::max(a, b), c);
		}
		
		// 向量与矩阵的乘法
		template <typename T>
		Vector_T<typename T::value_type, 4> transform(T const & v, Matrix4_T<typename T::value_type> const & mat) noexcept;
		
		template <typename T>
		T transform_coord(T const & v, Matrix4_T<typename T::value_type> const & mat) noexcept;

		//返回 from 与 to 之间的角度
		template<typename T>
		typename T::value_type Angle(const T& from, const T& to);
		
		int32_t SignBit(int32_t x) noexcept;
		float SignBit(float x) noexcept;

		// Color space
		///////////////////////////////////////////////////////////////////////////////
		float linear_to_srgb(float linear) noexcept;
		float srgb_to_linear(float srgb) noexcept;
		
		//四元数乘法
		template <typename T>
		Quaternion_T<T> mul(const Quaternion_T<T>&lhs, const Quaternion_T<T>&rhs) noexcept;

		//四元数共轭
		template <typename T>
		Quaternion_T<T> conjugate(const Quaternion_T<T>& rhs) noexcept;
		
		template <typename T>
		std::pair<Quaternion_T<T>, Quaternion_T<T>> inverse(const Quaternion_T<T>& real, const Quaternion_T<T>& dual) noexcept;

		//四元数的逆
		template <typename T>
		Quaternion_T<T> inverse(const Quaternion_T<T>& rhs) noexcept;

		template <typename T>
		std::pair<Quaternion_T<T>, Quaternion_T<T>> inverse(const Quaternion_T<T>& real, const Quaternion_T<T>& dual) noexcept;

		template <typename T>
		Quaternion_T<T> slerp(Quaternion_T<T> const & lhs, Quaternion_T<T> const & rhs, T s) noexcept;

		//矩形平移
		template<typename T>
		Matrix4_T<T> translation(T X, T Y, T Z);
		template<typename T>
		Matrix4_T<T> translation(const Vector_T<T, 3>& Move);

		// 矩形缩放
		template<typename T>
		Matrix4_T<T> MatrixScale(T X, T Y, T Z);
		template<typename T>
		Matrix4_T<T> MatrixScale(const Vector_T<T, 3>& Scale);

		//矩阵旋转
		template<typename T>
		Matrix4_T<T> rotation_x(T Angle);
		template<typename T>
		Matrix4_T<T> rotation_y(T Angle);
		template<typename T>
		Matrix4_T<T> rotation_z(T Angle);
		template <typename T>
		Matrix4_T<T> rotation_matrix_yaw_pitch_roll(const T& yaw, const T& pitch, const T& roll) noexcept;

		template <typename T>
		Matrix4_T<T> scaling(const T& sx, const T& sy, const T& sz) noexcept;
		template<typename T>
		Matrix4_T<T> scaling(const Vector_T<T, 3>& s) noexcept;

		//矩形乘法
		template<typename T>
		Matrix4_T<T> mul(const Matrix4_T<T>&lhs, const Matrix4_T<T>& rhs);

		//矩阵转置
		template<typename T>
		Matrix4_T<T> transpose(const Matrix4_T<T>& m);

		//矩阵的行列式
		template<typename T>
		T Determinant(const Matrix4_T<T>& m);

		//矩阵的逆
		template<typename T>
		Matrix4_T<T> inverse(const Matrix4_T<T>& m);

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
		void decompose(Vector_T<T, 3>& scale, Quaternion_T<T>& rot, Vector_T<T, 3>& trans, const Matrix4_T<T>& m);
	
		//相互转换
		template<typename T>
		Matrix4_T<T> to_matrix(const Quaternion_T<T>& quat);
		template<typename T>
		Matrix4_T<T> to_matrix(const Rotator_T<T>& rot);

		template<typename T>
		Quaternion_T<T> to_quaternion(const Matrix4_T<T>& mat);
		template <typename T>
		Quaternion_T<T> to_quaternion(Vector_T<T, 3> const & tangent, Vector_T<T, 3> const & binormal, Vector_T<T, 3> const & normal, uint32_t bits) noexcept;
		template<typename T>
		Quaternion_T<T> ToQuaternion(const Rotator_T<T>& rot);

		//template<typename T>
		//Rotator_T<T> ToRotator(const Matrix4_T<T>& mat);
		template<typename T>
		Rotator_T<T> ToRotator(const Quaternion_T<T>& quat);
		template<typename T>
		void ToYawPitchRoll(T& yaw, T& pitch, T& roll, const Quaternion_T<T>& quat);

		template <typename T>
		Quaternion_T<T> mul_real(const  Quaternion_T<T>& lhs_real, const  Quaternion_T<T>& rhs_real) noexcept;

		template <typename T>
		Quaternion_T<T> mul_dual(const  Quaternion_T<T>& lhs_real, const  Quaternion_T<T>& lhs_dual,
			const  Quaternion_T<T>& rhs_real, const  Quaternion_T<T>& rhs_dual) noexcept;


		// Dual quaternion
		///////////////////////////////////////////////////////////////////////////////
		template <typename T>
		Quaternion_T<T> quat_trans_to_udq(const Quaternion_T<T>&q, Vector_T<T, 3> const & t) noexcept;

		template <typename T>
		Vector_T<T, 3> udq_to_trans(const  Quaternion_T<T>& real, const  Quaternion_T<T>& dual) noexcept;

		template <typename T>
		Matrix4_T<T> udq_to_matrix(const Quaternion_T<T>&real, const Quaternion_T<T>&dual) noexcept;

		template <typename T>
		std::pair<Quaternion_T<T>, Quaternion_T<T>> conjugate(Quaternion_T<T> const & real, Quaternion_T<T> const & dual) noexcept;

		template <typename T>
		Matrix4_T<T> transformation(const Vector_T<T, 3>* scaling_center, const Quaternion_T<T>* scaling_rotation, const Vector_T<T, 3>* scale,
			const Vector_T<T, 3>* rotation_center, const Quaternion_T<T>* rotation, const Vector_T<T, 3>* trans) noexcept;

		template <typename T>
		void udq_to_screw(T& angle, T& pitch, Vector_T<T, 3>& dir, Vector_T<T, 3>& moment,
			Quaternion_T<T> const & real, Quaternion_T<T> const & dual) noexcept;

		template <typename T>
		std::pair<Quaternion_T<T>, Quaternion_T<T>> udq_from_screw(T const & angle, T const & pitch,
			Vector_T<T, 3> const & dir, Vector_T<T, 3> const & moment) noexcept;

		template <typename T>
		std::pair<Quaternion_T<T>, Quaternion_T<T>> sclerp(Quaternion_T<T> const & lhs_real, Quaternion_T<T> const & lhs_dual,
			Quaternion_T<T> const & rhs_real, Quaternion_T<T> const & rhs_dual, T s) noexcept;

		// 范围
		///////////////////////////////////////////////////////////////////////////////
		template <typename Iterator>
		AABBox_T<typename std::iterator_traits<Iterator>::value_type::value_type> compute_aabbox(Iterator first, Iterator last) noexcept;

		
		template <typename T>
		AABBox_T<T> transform_aabb(AABBox_T<T> const & aabb, Matrix4_T<T> const & mat) noexcept;
		template <typename T>
		AABBox_T<T> transform_aabb(AABBox_T<T> const & aabb, Vector_T<T, 3> const & scale, Quaternion_T<T> const & rot, Vector_T<T, 3> const & trans) noexcept;

		// 网格
		///////////////////////////////////////////////////////////////////////////////
		// 计算TBN基
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

			size_t const num = static_cast<size_t>(std::distance(xyzsBegin, xyzsEnd));

			for (size_t i = 0; i < num; ++ i)
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

				Vector_T<value_type, 3> const v1v0 = v1XYZ - v0XYZ;
				Vector_T<value_type, 3> const v2v0 = v2XYZ - v0XYZ;

				texcoord_type const & v0Tex(*(texsBegin + v0Index));
				texcoord_type const & v1Tex(*(texsBegin + v1Index));
				texcoord_type const & v2Tex(*(texsBegin + v2Index));

				value_type const s1 = v1Tex.x() - v0Tex.x();
				value_type const t1 = v1Tex.y() - v0Tex.y();

				value_type const s2 = v2Tex.x() - v0Tex.x();
				value_type const t2 = v2Tex.y() - v0Tex.y();

				value_type const denominator = s1 * t2 - s2 * t1;
				Vector_T<value_type, 3> tangent, binormal;
				if (MathWorker::abs(denominator) < std::numeric_limits<value_type>::epsilon())
				{
					tangent = Vector_T<value_type, 3>(1, 0, 0);
					binormal = Vector_T<value_type, 3>(0, 1, 0);
				}
				else
				{
					tangent = (t2 * v1v0 - t1 * v2v0) / denominator;
					binormal = (s1 * v2v0 - s2 * v1v0) / denominator;
				}

				*(targentsBegin + v0Index) += tangent;
				*(binormsBegin + v0Index) += binormal;

				*(targentsBegin + v1Index) += tangent;
				*(binormsBegin + v1Index) += binormal;

				*(targentsBegin + v2Index) += tangent;
				*(binormsBegin + v2Index) += binormal;
			}

			for (size_t i = 0; i < num; ++ i)
			{
				tangent_type tangent(*(targentsBegin + i));
				binormal_type binormal(*(binormsBegin + i));
				normal_type const normal(*(normalsBegin + i));

				// Gram-Schmidt orthogonalize
				tangent = normalize(tangent - normal * dot(tangent, normal));
				*(targentsBegin + i) = tangent;

				binormal_type binormal_cross = cross(normal, tangent);
				// Calculate handedness
				if (dot(binormal_cross, binormal) < 0)
				{
					binormal_cross = -binormal_cross;
				}

				*(binormsBegin + i) = binormal_cross;
			}
		}

		template <typename NormalIterator, typename IndexIterator, typename PositionIterator>
		inline void
		compute_normal(NormalIterator normalBegin,
								IndexIterator indicesBegin, IndexIterator indicesEnd,
								PositionIterator xyzsBegin, PositionIterator xyzsEnd) noexcept
		{
			typedef typename std::iterator_traits<PositionIterator>::value_type position_type;
			typedef typename std::iterator_traits<NormalIterator>::value_type normal_type;
			typedef typename position_type::value_type value_type;

			NormalIterator normalEnd = normalBegin;
			std::advance(normalEnd, std::distance(xyzsBegin, xyzsEnd));
			std::fill(normalBegin, normalEnd, normal_type::Zero());

			for (IndexIterator iter = indicesBegin; iter != indicesEnd; iter += 3)
			{
				uint32_t const v0Index = *(iter + 0);
				uint32_t const v1Index = *(iter + 1);
				uint32_t const v2Index = *(iter + 2);

				position_type const & v0(*(xyzsBegin + v0Index));
				position_type const & v1(*(xyzsBegin + v1Index));
				position_type const & v2(*(xyzsBegin + v2Index));

				Vector_T<value_type, 3> v03(v0.x(), v0.y(), v0.z());
				Vector_T<value_type, 3> v13(v1.x(), v1.y(), v1.z());
				Vector_T<value_type, 3> v23(v2.x(), v2.y(), v2.z());

				Vector_T<value_type, 3> vec(cross(v13 - v03, v23 - v03));

				*(normalBegin + v0Index) += vec;
				*(normalBegin + v1Index) += vec;
				*(normalBegin + v2Index) += vec;
			}

			for (NormalIterator iter = normalBegin; iter != normalEnd; ++ iter)
			{
				*iter = normalize(*iter);
			}
		}
	}
}

#include <math/vectorxd.h>
#include <math/matrix.h>
#include <math/quaternion.h>
#include <math/rotator.h>
#include <math/color.h>
#include <math/half.h>
#include <math/AABBox.h>
