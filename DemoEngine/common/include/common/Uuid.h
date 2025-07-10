#pragma once
#include <common/macro.h>
#include <array>
#include <cstring>

#ifdef ZENGINE_PLATFORM_WINDOWS
#include <guiddef.h>
#endif

namespace CommonWorker
{

    struct Uuid
    {
        uint32_t data1;
        uint16_t data2;
        uint16_t data3;
        uint8_t data4[8];

        Uuid() noexcept = default;
        constexpr Uuid(uint32_t d1, uint16_t d2, uint16_t d3, std::array<uint8_t, 8> const& d4) noexcept
            : data1(d1), data2(d2), data3(d3), data4{d4[0], d4[1], d4[2], d4[3], d4[4], d4[5], d4[6], d4[7]}
        {
        }

#ifdef ZENGINE_PLATFORM_WINDOWS
        constexpr Uuid(GUID const& value) noexcept
            : Uuid(value.Data1, value.Data2, value.Data3,
                {{value.Data4[0], value.Data4[1], value.Data4[2], value.Data4[3], value.Data4[4], value.Data4[5], value.Data4[6],
                    value.Data4[7]}})
        {
        }

        operator GUID const &() const noexcept
        {
            return reinterpret_cast<GUID const&>(*this);
        }
#endif
    };

    inline bool operator==(Uuid const& lhs, Uuid const& rhs) noexcept
    {
        return !std::memcmp(&lhs, &rhs, sizeof(Uuid));
    }

    inline bool operator!=(Uuid const& lhs, Uuid const& rhs) noexcept
    {
        return !(lhs == rhs);
    }

    template <typename T>
    Uuid const& UuidOf();

    template <typename T>
    inline Uuid const& UuidOf([[maybe_unused]] T* p)
    {
        return UuidOf<T>();
    }

    template <typename T>
    inline Uuid const& UuidOf([[maybe_unused]] T const* p)
    {
        return UuidOf<T>();
    }

    template <typename T>
    inline Uuid const& UuidOf([[maybe_unused]] T& p)
    {
        return UuidOf<T>();
    }

    template <typename T>
    inline Uuid const& UuidOf([[maybe_unused]] T const& p)
    {
        return UuidOf<T>();
    }

}


#define DEFINE_UUID_OF(x)                                      \
    template <>                                                \
    RenderWorker::Uuid const& RenderWorker::UuidOf<x>()                    \
    {                                                          \
        return reinterpret_cast<RenderWorker::Uuid const&>(IID_##x); \
    }
