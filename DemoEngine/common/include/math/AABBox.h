#pragma once
#include <math/vectorxd.h>

namespace RenderWorker
{
template <typename T>
class AABBox_T
{
public:
    constexpr AABBox_T() noexcept
    {
    }
    AABBox_T(Vector_T<T, 3> vMin, Vector_T<T, 3> vMax) noexcept;
    constexpr AABBox_T(AABBox_T<T> const& rhs) noexcept : min_(rhs.min_), max_(rhs.max_)
    {
    }
    constexpr AABBox_T(AABBox_T<T>&& rhs) noexcept : min_(std::move(rhs.min_)), max_(std::move(rhs.max_))
    {
    }

    constexpr Vector_T<T, 3>& Min() noexcept
    {
        return min_;
    }
    constexpr Vector_T<T, 3> const & Min() const noexcept
    {
        return min_;
    }
    constexpr Vector_T<T, 3>& Max() noexcept
    {
        return max_;
    }
    constexpr Vector_T<T, 3> const & Max() const noexcept
    {
        return max_;
    }

    AABBox_T<T>& operator=(const AABBox_T<T>& rhs) noexcept;
	AABBox_T<T>& operator=(AABBox_T<T>&& rhs) noexcept;
private:
	Vector_T<T, 3> min_, max_;
};



using AABBox = AABBox_T<float>;
}