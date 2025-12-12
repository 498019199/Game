#include <math/AABBox.h>

namespace RenderWorker
{
template <typename T>
AABBox_T<T>::AABBox_T(Vector_T<T, 3> vMin, Vector_T<T, 3> vMax) noexcept
    : min_(std::move(vMin)), max_(std::move(vMax))
{
    COMMON_ASSERT(min_.x() <= max_.x());
    COMMON_ASSERT(min_.y() <= max_.y());
    COMMON_ASSERT(min_.z() <= max_.z());
}

template<typename T>
AABBox_T<T>& AABBox_T<T>::operator=(const AABBox_T<T>& rhs) noexcept
{
    if (this != &rhs)
    {
        this->Min() = rhs.Min();
        this->Max() = rhs.Max();
    }
    return *this;
}

template<typename T>
AABBox_T<T>& AABBox_T<T>::operator=(AABBox_T<T>&& rhs) noexcept
{
    min_ = std::move(rhs.min_);
    max_ = std::move(rhs.max_);
    return *this;
}

template class AABBox_T<float>;
}