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

private:
	uint16_t value_{};
};
}