#include <math/Frustum.h>

namespace RenderWorker
{
namespace
{
template <typename T>
T PlaneDistance(Vector_T<T, 4> const& plane, Vector_T<T, 3> const& point) noexcept
{
    return plane.x() * point.x() + plane.y() * point.y() + plane.z() * point.z() + plane.w();
}

template <typename T>
Vector_T<T, 4> NormalizePlane(Vector_T<T, 4> const& plane) noexcept
{
    T const length = MathWorker::sqrt(
        plane.x() * plane.x() + plane.y() * plane.y() + plane.z() * plane.z());
    if (length <= std::numeric_limits<T>::epsilon())
    {
        return plane;
    }
    return plane / length;
}
}

template <typename T>
void Frustum_T<T>::ClipMatrix(Matrix4_T<T> const& clip, Matrix4_T<T> const& inv_clip) noexcept
{
    // Direct3D-style clip space: x/y in [-1, 1], z in [0, 1].
    corners_[0] = MathWorker::transform_coord(CornerType(-1, -1, 0), inv_clip); // left bottom near
    corners_[1] = MathWorker::transform_coord(CornerType(+1, -1, 0), inv_clip); // right bottom near
    corners_[2] = MathWorker::transform_coord(CornerType(-1, +1, 0), inv_clip); // left top near
    corners_[3] = MathWorker::transform_coord(CornerType(+1, +1, 0), inv_clip); // right top near
    corners_[4] = MathWorker::transform_coord(CornerType(-1, -1, 1), inv_clip); // left bottom far
    corners_[5] = MathWorker::transform_coord(CornerType(+1, -1, 1), inv_clip); // right bottom far
    corners_[6] = MathWorker::transform_coord(CornerType(-1, +1, 1), inv_clip); // left top far
    corners_[7] = MathWorker::transform_coord(CornerType(+1, +1, 1), inv_clip); // right top far

    Plane const column1 = clip.Col(0);
    Plane const column2 = clip.Col(1);
    Plane const column3 = clip.Col(2);
    Plane const column4 = clip.Col(3);

    planes_[0] = NormalizePlane(column4 - column1); // right
    planes_[1] = NormalizePlane(column4 + column1); // left
    planes_[2] = NormalizePlane(column4 - column2); // top
    planes_[3] = NormalizePlane(column4 + column2); // bottom
    planes_[4] = NormalizePlane(column4 - column3); // far
    planes_[5] = NormalizePlane(column3);           // near
}

template <typename T>
bool Frustum_T<T>::VecInBound(CornerType const& point) const noexcept
{
    for (Plane const& plane : planes_)
    {
        if (PlaneDistance(plane, point) < 0)
        {
            return false;
        }
    }
    return true;
}

template <typename T>
BoundOverlap Frustum_T<T>::Intersect(AABBox_T<T> const& aabb) const noexcept
{
    bool partial = false;
    for (Plane const& plane : planes_)
    {
        CornerType positive(
            plane.x() >= 0 ? aabb.Max().x() : aabb.Min().x(),
            plane.y() >= 0 ? aabb.Max().y() : aabb.Min().y(),
            plane.z() >= 0 ? aabb.Max().z() : aabb.Min().z());
        if (PlaneDistance(plane, positive) < 0)
        {
            return BoundOverlap::No;
        }

        CornerType negative(
            plane.x() >= 0 ? aabb.Min().x() : aabb.Max().x(),
            plane.y() >= 0 ? aabb.Min().y() : aabb.Max().y(),
            plane.z() >= 0 ? aabb.Min().z() : aabb.Max().z());
        partial |= PlaneDistance(plane, negative) < 0;
    }
    return partial ? BoundOverlap::Partial : BoundOverlap::Yes;
}

template class Frustum_T<float>;
}

