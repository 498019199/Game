// 半精度浮点数（Half-precision floating-point）
// 图形和游戏开发：在实时图形渲染中，对于一些对精度要求不是特别高的计算，如某些纹理数据、顶点属性等，可以使用半精度来节省内存和带宽。
// 比如，在移动游戏开发中，为了在有限的资源下实现较好的性能，会采用半精度浮点数。
//---------------------------------------------------------------------------
//
//	half -- a 16-bit floating point number class:
//
//	Type half can represent positive and negative numbers whose
//	magnitude is between roughly 6.1e-5 and 6.5e+4 with a relative
//	error of 9.8e-4; numbers smaller than 6.1e-5 can be represented
//	with an absolute error of 6.0e-8.  All integers from -2048 to
//	+2048 can be represented exactly.
//
//	Type half behaves (almost) like the built-in C++ floating point
//	types.  In arithmetic expressions, half, float and double can be
//	mixed freely.  Here are a few examples:
//
//	    half a (3.5);
//	    float b (a + sqrt (a));
//	    a += b;
//	    b += a;
//	    b = a + 7;
//
//	Conversions from half to float are lossless; all half numbers
//	are exactly representable as floats.
//
//	Conversions from float to half may not preserve a float's value
//	exactly.  If a float is not representable as a half, then the
//	float value is rounded to the nearest representable half.  If a
//	float value is exactly in the middle between the two closest
//	representable half values, then the float value is rounded to
//	the closest half whose least significant bit is zero.
//
//	Overflows during float-to-half conversions cause arithmetic
//	exceptions.  An overflow occurs when the float value to be
//	converted is too large to be represented as a half, or if the
//	float value is an infinity or a NAN.
//
//	The implementation of type half makes the following assumptions
//	about the implementation of the built-in C++ types:
//
//	    float is an IEEE 754 single-precision number
//	    sizeof (float) == 4
//	    sizeof (unsigned int) == sizeof (float)
//	    alignof (unsigned int) == alignof (float)
//	    sizeof (unsigned short) == 2
//
//---------------------------------------------------------------------------
#pragma once
#include <math/vectorxd.h>

#define HALF_MIN	5.96046448e-08f	// Smallest positive half

#define HALF_NRM_MIN	6.10351562e-05f	// Smallest positive normalized half

#define HALF_MAX	65504.0f	// Largest positive half

#define HALF_EPSILON	0.00097656f	// Smallest positive e for which
					// half (1.0 + e) != half (1.0)

#define HALF_MANT_DIG	11		// Number of digits in mantissa
					// (significand + hidden leading 1)

#define HALF_DIG	2		// Number of base 10 digits that
					// can be represented without change

#define HALF_RADIX	2		// Base of the exponent

#define HALF_MIN_EXP	-13		// Minimum negative integer such that
					// HALF_RADIX raised to the power of
					// one less than that integer is a
					// normalized half

#define HALF_MAX_EXP	16		// Maximum positive integer such that
					// HALF_RADIX raised to the power of
					// one less than that integer is a
					// normalized half

#define HALF_MIN_10_EXP	-4		// Minimum positive integer such
					// that 10 raised to that power is
					// a normalized half

#define HALF_MAX_10_EXP	4		// Maximum positive integer such
					// that 10 raised to that power is
					// a normalized half

namespace RenderWorker
{
// 不知道这个类干什么的，跟颜色有关
class half
{
public:
    constexpr half() noexcept
    {
    }
    explicit half(float f) noexcept;
    half(half const& rhs) noexcept
        : value_(rhs.value_)
    {
    }

    operator float() const noexcept;

    // 特殊值
    // returns +infinity
    static half pos_inf() noexcept;
    // returns -infinity
    static half neg_inf() noexcept;
    // returns a NAN with the bit pattern 0111111111111111
    static half q_nan() noexcept;
    // returns a NAN with the bit pattern 0111110111111111
    static half s_nan() noexcept;
private:
	uint16_t value_{};
};
}