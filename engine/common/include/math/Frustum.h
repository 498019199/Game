#pragma once

#include <math/math.h>

#include <array>

namespace RenderWorker
{
template <typename T>
class Frustum_T final
{
public:
    using Plane = Vector_T<T, 4>;
    using CornerType = Vector_T<T, 3>;

    void ClipMatrix(Matrix4_T<T> const& clip, Matrix4_T<T> const& inv_clip) noexcept;

    constexpr Plane const& FrustumPlane(uint32_t index) const noexcept
    {
        return planes_[index];
    }

    constexpr CornerType const& Corner(uint32_t index) const noexcept
    {
        return corners_[index];
    }

    bool VecInBound(CornerType const& point) const noexcept;
    BoundOverlap Intersect(AABBox_T<T> const& aabb) const noexcept;

private:
    std::array<Plane, 6> planes_ {};
    std::array<CornerType, 8> corners_ {};
};

using Frustum = Frustum_T<float>;
}
