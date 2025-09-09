#pragma once
#include <common/macro.h>

#include <cassert>
#include <cstdint>
#include <memory>

#include <string>
#include <string_view>
#include <bit>
#include <utility>

#ifdef ZENGINE_DEBUG
	#define COMMON_ASSERT(val) assert(val)
#else
	#define COMMON_ASSERT(val) 
#endif//ZENGINE_DEBUG

#if defined(ZENGINE_CXX23_LIBRARY_TO_UNDERLYING_SUPPORT)
#include <utility>
#else
#include <type_traits>

#define KFL_UNUSED(x) (void)(x)

namespace std
{
	template <typename T>
	constexpr std::underlying_type_t<T> to_underlying(T e) noexcept
	{
		return static_cast<std::underlying_type_t<T>>(e);
	}
} // namespace std
#endif

#if defined(ZENGINE_CXX23_LIBRARY_UNREACHABLE_SUPPORT)
    #include <utility>
#else
    namespace std
    {
        [[noreturn]] inline void unreachable()
        {
    #if defined(ZENGINE_COMPILER_MSVC)
            __assume(false);
    #else
            __builtin_unreachable();
    #endif
        }
    } // namespace std
#endif


#if defined(ZENGINE_DEBUG)
    #define _CRTDBG_MAP_ALLOC
    #include <crtdbg.h>
    #define ZENGINE_DBG_SUFFIX "_d"
#else
    #define ZENGINE_DBG_SUFFIX ""
#endif
#define DO_STRINGIZE(X) #X
#define ZENGINE_OUTPUT_SUFFIX  "_" DO_STRINGIZE(ZENGINE_COMPILER_NAME) DO_STRINGIZE(ZENGINE_COMPILER_VERSION) ZENGINE_DBG_SUFFIX

namespace CommonWorker
{

    std::string& Convert(std::string& dest, std::wstring_view src);
    std::wstring& Convert(std::wstring& dest, std::string_view src);

    // 产生FourCC常量
    template <unsigned char ch0, unsigned char ch1, unsigned char ch2, unsigned char ch3>
    struct MakeFourCC
    {
        static uint32_t constexpr value = (ch0 << 0) + (ch1 << 8) + (ch2 << 16) + (ch3 << 24);
    };

    // Endian的转换
    template <int size>
    void EndianSwitch(void* p) noexcept;

    template <typename T>
    T Native2BE(T x) noexcept
    {
        if constexpr (std::endian::native == std::endian::little)
        {
            EndianSwitch<sizeof(T)>(&x);
        }
        return x;
    }
    template <typename T>
    T Native2LE(T x) noexcept
    {
        if constexpr (std::endian::native == std::endian::big)
        {
            EndianSwitch<sizeof(T)>(&x);
        }
        return x;
    }

    template <typename T>
    T BE2Native(T x) noexcept
    {
        return Native2BE(x);
    }
    template <typename T>
    T LE2Native(T x) noexcept
    {
        return Native2LE(x);
    }

    template <typename T, typename... Args>
    inline std::shared_ptr<T> MakeSharedPtr(Args&&... args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
    inline std::unique_ptr<T> MakeUniquePtrHelper(std::false_type, Args&&... args)
    {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
    inline std::unique_ptr<T> MakeUniquePtrHelper(std::true_type, size_t size)
    {
        static_assert(0 == std::extent<T>::value,
            "make_unique<T[N]>() is forbidden, please use make_unique<T[]>().");

        return std::make_unique<T>(size);
    }

    template <typename T, typename... Args>
    inline std::unique_ptr<T> MakeUniquePtr(Args&&... args)
    {
        return MakeUniquePtrHelper<T>(std::is_array<T>(), std::forward<Args>(args)...);
    }


    template <typename To, typename From>
    inline To checked_cast(From* p) noexcept
    {
        COMMON_ASSERT(dynamic_cast<To>(p) == static_cast<To>(p));
        return static_cast<To>(p);
    }

    template <typename To, typename From>
    inline To checked_cast(From const* p) noexcept
    {
        COMMON_ASSERT(dynamic_cast<To>(p) == static_cast<To>(p));
        return static_cast<To>(p);
    }

    template <typename To, typename From>
    inline typename std::add_lvalue_reference<To>::type checked_cast(From& p) noexcept
    {
        typedef typename std::remove_reference<To>::type RawToType;
        COMMON_ASSERT(dynamic_cast<RawToType*>(&p) == static_cast<RawToType*>(&p));
        return static_cast<RawToType&>(p);
    }

    template <typename To, typename From>
    inline typename std::add_lvalue_reference<const To>::type checked_cast(const From& p) noexcept
    {
        typedef typename std::remove_reference<const To>::type RawToType;
        COMMON_ASSERT(dynamic_cast<const RawToType*>(&p) == static_cast<const RawToType*>(&p));
        return static_cast<const RawToType&>(p);
    }

    template <typename To, typename From>
    inline std::shared_ptr<To>
    checked_pointer_cast(std::shared_ptr<From> const & p) noexcept
    {
        COMMON_ASSERT(dynamic_cast<To*>(p.get()) == static_cast<To*>(p.get()));
        return std::static_pointer_cast<To>(p);
    }

    class ResIdentifier;
    std::string ReadShortString(ResIdentifier& res);
    void WriteShortString(std::ostream& os, std::string_view str);
}