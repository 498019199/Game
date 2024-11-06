#include <math/half.h>
#include <bit>

namespace MathWorker
{
half::half(float f) noexcept
{
    int32_t i = std::bit_cast<int32_t>(f);

    int32_t s = (i >> 16) & 0x00008000;
    int32_t e = ((i >> 23) & 0x000000FF) - (127 - 15);
    int32_t m = i & 0x007FFFFF;

    if (e <= 0)
    {
        if (e < -10)
        {
            value_ = 0;
        }
        else
        {
            m = (m | 0x00800000) >> (1 - e);

            if (m &  0x00001000)
            {
                m += 0x00002000;
            }

            value_ = static_cast<uint16_t>(s | (m >> 13));
        }
    }
    else
    {
        if (0xFF - (127 - 15) == e)
        {
            e = 31;
        }
        else
        {
            if (m & 0x00001000)
            {
                m += 0x00002000;

                if (m & 0x00800000)
                {
                    m = 0;		// overflow in significand,
                    e += 1;		// adjust exponent
                }
            }
        }

        value_ = static_cast<uint16_t>(s | (e << 10) | (m >> 13));
    }
}

half::operator float() const noexcept
{
    int32_t ret;

    int32_t s = ((value_ & 0x8000) >> 15) << 31;
    int32_t e = (value_ & 0x7C00) >> 10;
    int32_t m = value_ & 0x03FF;

    if (0 == e)
    {
        if (m != 0)
        {
            // Denormalized number -- renormalize it

            while (!(m & 0x00000400))
            {
                m <<= 1;
                e -= 1;
            }

            e += 1;
            m &= ~0x00000400;
        }
    }
    else
    {
        if (31 == e)
        {
            if (m != 0)
            {
                // Nan -- preserve sign and significand bits
                e = 0xFF - (127 - 15);
            }
        }
    }

    // Normalized number
    e += 127 - 15;
    m <<= 13;

    ret = s | (e << 23) | m;

    return std::bit_cast<float>(ret);
}

}