#ifndef _MATH_HELPER_H_
#define _MATH_HELPER_H_
#pragma once

// Ä£°åµÝ¹é
namespace MathHelper
{
	// dot help
	template<typename T, int N>
	struct dot_helper
	{
		static T Do(const T* lhs, const T* rhs)
		{
			return lhs[0] * rhs[0] + dot_helper<T, N - 1>::Do(lhs + 1, rhs + 1);
		}
	};

	template<typename T>
	struct dot_helper<T, 1>
	{
		static T Do(const T* lhs, const T* rhs)
		{
			return lhs[0] * rhs[0];
		}
	};

	// vector help
    template<typename T, int N>
    struct vector_helper
    {
		template<typename U>
		static void DoCopy(T out[N], const U rhs[N]) noexcept
		{
			out[0] = static_cast<T>(rhs[0]);
            vector_helper<T, N - 1>::DoCopy(out + 1, rhs + 1);
		}

		static void DoAssign(T out[N], const T rhs) noexcept
		{
			out[0] = rhs;
            vector_helper<T, N - 1>::DoAssign(out + 1, rhs);
		}

		static void DoAdd(T out[N], const T lhs[N], const T rhs[N]) noexcept
        {
            out[0] = lhs[0] + rhs[0];
            vector_helper<T, N - 1>::DoAdd(out + 1, lhs + 1, rhs + 1);
        }

		static void DoSub(T out[N], const T lhs[N], const T rhs[N]) noexcept
        {
            out[0] = lhs[0] - rhs[0];
            vector_helper<T, N - 1>::DoSub(out + 1, lhs + 1, rhs + 1);
        }

		static void DoMul(T out[N], const T lhs[N], const T rhs[N]) noexcept
        {
            out[0] = lhs[0] * rhs[0];
            vector_helper<T, N - 1>::DoMul(out + 1, lhs + 1, rhs + 1);
        }

		static void DoScale(T out[N], const T lhs[N], const T rhs) noexcept
        {
            out[0] = lhs[0] * rhs;
            vector_helper<T, N - 1>::DoScale(out + 1, lhs + 1, rhs);
        }

		static void DoDiv(T out[N], const T lhs[N], const T rhs[N]) noexcept
        {
            out[0] = lhs[0] / rhs[0];
            vector_helper<T, N - 1>::DoDiv(out + 1, lhs + 1, rhs + 1);
        }

		static void DoNegate(T out[N], const T rhs[N]) noexcept
        { 
            out[0] = -rhs[0];
            vector_helper<T, N - 1>::DoNegate(out + 1, rhs + 1);
        }

		static void DoSwap(T out[N], T rhs[N]) noexcept
        {
            std::swap(out[0], rhs[0]);
            vector_helper<T, N - 1>::DoSwap(out + 1, rhs + 1);
        }

		static bool DoEquip(const T out[N], const T rhs[N]) noexcept
        {
            return vector_helper<T, 1>::DoEquip(out, rhs) && vector_helper<T, N - 1>::DoEquip(out + 1, rhs + 1);
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

		static void DoAssign(T out[1], const T rhs) noexcept
		{
			out[0] = rhs;
		}

		static void DoAdd(T out[1], const T lhs[1], const T rhs[1]) noexcept
		{
			out[0] = lhs[0] + rhs[0];
		}

		static void DoSub(T out[1], const T lhs[1], const T rhs[1]) noexcept
		{
			out[0] = lhs[0] - rhs[0];
		}

		static void DoMul(T out[1], const T lhs[1], const T rhs[1]) noexcept
		{
			out[0] = lhs[0] * rhs[0];
		}

		static void DoScale(T out[1], const T lhs[1], const T rhs) noexcept
		{
			out[0] = lhs[0] * rhs;
		}

		static void DoDiv(T out[1], const T lhs[1], const T rhs[1]) noexcept
		{
			out[0] = lhs[0] / rhs[0];
		}

		static void DoNegate(T out[1], const T rhs[1]) noexcept
		{
			out[0] = -rhs[0];
		}

		static void DoSwap(T out[1], T rhs[1]) noexcept
		{
			std::swap(out[0], rhs[0]);
		}

		static bool DoEquip(const T out[1], const T rhs[1]) noexcept
		{
            return (out[0] == rhs[0]);
		}
	};

	template<typename T, int N>
	struct MaxMinimizeHelper
	{
		static void DoMax(T out[N], T const lhs[N], T const rhs[N]) noexcept
		{
			out[0] = std::max<T>(lhs[0], rhs[0]);
			MaxMinimizeHelper<T, N - 1>::DoMax(out + 1, lhs + 1, rhs + 1);
		}

		static void DoMin(T out[N], T const lhs[N], T const rhs[N]) noexcept
		{
			out[0] = std::min<T>(lhs[0], rhs[0]);
			MaxMinimizeHelper<T, N - 1>::DoMin(out + 1, lhs + 1, rhs + 1);
		}
	};

	template<typename T>
	struct MaxMinimizeHelper<T, 1>
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
};

#endif//_MATH_HELPER_H_
