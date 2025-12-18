#pragma once

namespace RenderWorker
{
	template <typename T, size_t N>
	class Vector_T;
	template <typename T>
	class Matrix4_T;

	namespace MathHelper
	{
		// dot help
		template <typename T, size_t N>
		struct dot_helper
		{
			static T Do(T const * lhs, T const * rhs) noexcept
			{
				return lhs[0] * rhs[0] + dot_helper<T, N - 1>::Do(lhs + 1, rhs + 1);
			}
		};
		template <typename T>
		struct dot_helper<T, 1>
		{
			static T Do(T const * lhs, T const * rhs) noexcept
			{
				return lhs[0] * rhs[0];
			}
		};

		// vector help
		template<typename T, size_t N>
		struct vector_helper
		{
			template <typename U>
			static void DoCopy(T out[N], U const rhs[N]) noexcept
			{
				out[0] = static_cast<T>(rhs[0]);
				vector_helper<T, N - 1>::DoCopy(out + 1, rhs + 1);
			}

			static void DoSplat(T out[N], T const& rhs) noexcept
			{
				out[0] = rhs;
				vector_helper<T, N - 1>::DoSplat(out + 1, rhs);
			}

			template <typename U>
			static void DoAdd(T out[N], T const lhs[N], U const rhs[N]) noexcept
			{
				out[0] = lhs[0] + rhs[0];
				vector_helper<T, N - 1>::DoAdd(out + 1, lhs + 1, rhs + 1);
			}

			template <typename U>
			static void DoAdd(T out[N], T const lhs[N], U const & rhs) noexcept
			{
				out[0] = lhs[0] + rhs;
				vector_helper<T, N - 1>::DoAdd(out + 1, lhs + 1, rhs);
			}

			template <typename U>
			static void DoSub(T out[N], T const lhs[N], U const rhs[N]) noexcept
			{
				out[0] = lhs[0] - rhs[0];
				vector_helper<T, N - 1>::DoSub(out + 1, lhs + 1, rhs + 1);
			}

			template <typename U>
			static void DoSub(T out[N], T const lhs[N], U const & rhs) noexcept
			{
				out[0] = lhs[0] - rhs;
				vector_helper<T, N - 1>::DoSub(out + 1, lhs + 1, rhs);
			}

			template <typename U>
			static void DoMul(T out[N], T const lhs[N], U const rhs[N]) noexcept
			{
				out[0] = lhs[0] * rhs[0];
				vector_helper<T, N - 1>::DoMul(out + 1, lhs + 1, rhs + 1);
			}

			template <typename U>
			static void DoScale(T out[N], T const lhs[N], U const & rhs) noexcept
			{
				out[0] = lhs[0] * rhs;
				vector_helper<T, N - 1>::DoScale(out + 1, lhs + 1, rhs);
			}

			template <typename U>
			static void DoDiv(T out[N], T const lhs[N], U const rhs[N]) noexcept
			{
				out[0] = lhs[0] / rhs[0];
				vector_helper<T, N - 1>::DoDiv(out + 1, lhs + 1, rhs + 1);
			}

			static void DoNegate(T out[N], T const rhs[N]) noexcept
			{
				out[0] = -rhs[0];
				vector_helper<T, N - 1>::DoNegate(out + 1, rhs + 1);
			}

			static void DoSwap(T lhs[N], T rhs[N]) noexcept
			{
				std::swap(lhs[0], rhs[0]);
				return vector_helper<T, N - 1>::DoSwap(lhs + 1, rhs + 1);
			}

			static bool DoEqual(T const lhs[N], T const rhs[N]) noexcept
			{
				return vector_helper<T, 1>::DoEqual(lhs, rhs) && vector_helper<T, N - 1>::DoEqual(lhs + 1, rhs + 1);
			}
		};

		template<typename T>
		struct vector_helper<T, 1>
		{
			template<typename U>
			static void DoCopy(T out[1], const U rhs[1]) noexcept
			{
				out[0] = static_cast<T>(rhs[0]);
			}

			static void DoSplat(T out[1], T const& rhs) noexcept
			{
				out[0] = rhs;
			}

			template <typename U>
			static void DoAdd(T out[1], T const lhs[1], U const rhs[1]) noexcept
			{
				out[0] = lhs[0] + rhs[0];
			}

			template <typename U>
			static void DoAdd(T out[1], T const lhs[1], U const rhs) noexcept
			{
				out[0] = lhs[0] + rhs;
			}

			template <typename U>
			static void DoSub(T out[1], T const lhs[1], U const rhs[1]) noexcept
			{
				out[0] = lhs[0] - rhs[0];
			}

			template <typename U>
			static void DoSub(T out[1], T const lhs[1], U const & rhs) noexcept
			{
				out[0] = lhs[0] - rhs;
			}

			template <typename U>
			static void DoMul(T out[1], T const lhs[1], U const rhs[1]) noexcept
			{
				out[0] = lhs[0] * rhs[0];
			}

			template <typename U>
			static void DoScale(T out[1], T const lhs[1], U const & rhs) noexcept
			{
				out[0] = lhs[0] * rhs;
			}

			template <typename U>
			static void DoDiv(T out[1], T const lhs[1], U const rhs[1]) noexcept
			{
				out[0] = lhs[0] / rhs[0];
			}

			static void DoNegate(T out[1], T const rhs[1]) noexcept
			{
				out[0] = -rhs[0];
			}

			static bool DoEqual(T const lhs[1], T const rhs[1]) noexcept
			{
				return lhs[0] == rhs[0];
			}

			static void DoSwap(T lhs[1], T rhs[1]) noexcept
			{
				std::swap(lhs[0], rhs[0]);
			}
		};

		template<typename T, size_t N>
		struct max_minimize_helper
		{
			static void DoMax(T out[N], T const lhs[N], T const rhs[N]) noexcept
			{
				out[0] = std::max<T>(lhs[0], rhs[0]);
				max_minimize_helper<T, N - 1>::DoMax(out + 1, lhs + 1, rhs + 1);
			}

			static void DoMin(T out[N], T const lhs[N], T const rhs[N]) noexcept
			{
				out[0] = std::min<T>(lhs[0], rhs[0]);
				max_minimize_helper<T, N - 1>::DoMin(out + 1, lhs + 1, rhs + 1);
			}
		};

		template<typename T>
		struct max_minimize_helper<T, 1>
		{
			static void DoMax(T out[1], T const lhs[1], T const rhs[1]) noexcept
			{
				out[0] = std::max<T>(lhs[0], rhs[0]);
			}

			static void DoMin(T out[1], T const lhs[1], T const rhs[1]) noexcept
			{
				out[0] = std::min<T>(lhs[0], rhs[0]);
			}
		};

		template <typename T, size_t N>
		struct transform_helper
		{
			static Vector_T<T, 4> Do(const Vector_T<T, N> & v, const Matrix4_T<T> & mat) noexcept;
		};
		template <typename T>
		struct transform_helper<T, 4>
		{
			static Vector_T<T, 4> Do(const Vector_T<T, 4> & v, const Matrix4_T<T> & mat) noexcept
			{
				return Vector_T<T, 4>(v.x() * mat(0, 0) + v.y() * mat(1, 0) + v.z() * mat(2, 0) + v.w() * mat(3, 0),
					v.x() * mat(0, 1) + v.y() * mat(1, 1) + v.z() * mat(2, 1) + v.w() * mat(3, 1),
					v.x() * mat(0, 2) + v.y() * mat(1, 2) + v.z() * mat(2, 2) + v.w() * mat(3, 2),
					v.x() * mat(0, 3) + v.y() * mat(1, 3) + v.z() * mat(2, 3) + v.w() * mat(3, 3));
			}
		};
		template <typename T>
		struct transform_helper<T, 3>
		{
			static Vector_T<T, 4> Do(const Vector_T<T, 3> & v, const Matrix4_T<T> & mat) noexcept
			{
				return Vector_T<T, 4>(v.x() * mat(0, 0) + v.y() * mat(1, 0) + v.z() * mat(2, 0) + mat(3, 0),
					v.x() * mat(0, 1) + v.y() * mat(1, 1) + v.z() * mat(2, 1) + mat(3, 1),
					v.x() * mat(0, 2) + v.y() * mat(1, 2) + v.z() * mat(2, 2) + mat(3, 2),
					v.x() * mat(0, 3) + v.y() * mat(1, 3) + v.z() * mat(2, 3) + mat(3, 3));
			}
		};
		template <typename T>
		struct transform_helper<T, 2>
		{
			static Vector_T<T, 4> Do(const Vector_T<T, 2> & v, const Matrix4_T<T> & mat) noexcept
			{
				return Vector_T<T, 4>(v.x() * mat(0, 0) + v.y() * mat(1, 0) + mat(3, 0),
					v.x() * mat(0, 1) + v.y() * mat(1, 1) + mat(3, 1),
					v.x() * mat(0, 2) + v.y() * mat(1, 2) + mat(3, 2),
					v.x() * mat(0, 3) + v.y() * mat(1, 3) + mat(3, 3));
			}
		};
	}
}