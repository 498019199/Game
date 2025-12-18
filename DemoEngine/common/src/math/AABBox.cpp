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

template <typename T>
AABBox_T<T>& AABBox_T<T>::operator+=(Vector_T<T, 3> const & rhs) noexcept
{
    min_ += rhs;
    max_ += rhs;
    return *this;
}

template <typename T>
AABBox_T<T>& AABBox_T<T>::operator-=(Vector_T<T, 3> const & rhs) noexcept
{
    min_ -= rhs;
    max_ -= rhs;
    return *this;
}

template <typename T>
AABBox_T<T>& AABBox_T<T>::operator*=(T const& rhs) noexcept
{
    this->Min() *= rhs;
    this->Max() *= rhs;
    return *this;
}

template <typename T>
AABBox_T<T>& AABBox_T<T>::operator/=(T const& rhs) noexcept
{
    return this->operator*=(1.0f / rhs);
}

template <typename T>
AABBox_T<T>& AABBox_T<T>::operator&=(AABBox_T<T> const & rhs) noexcept
{
    min_ = MathWorker::maximize(this->Min(), rhs.Min());
    max_ = MathWorker::minimize(this->Max(), rhs.Max());
    return *this;
}

template <typename T>
AABBox_T<T>& AABBox_T<T>::operator|=(AABBox_T<T> const & rhs) noexcept
{
    min_ = MathWorker::minimize(this->Min(), rhs.Min());
    max_ = MathWorker::maximize(this->Max(), rhs.Max());
    return *this;
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

template <typename T>
Vector_T<T, 3> AABBox_T<T>::Center() const noexcept
{
    return (min_ + max_) / 2.0f;
}

template <typename T>
Vector_T<T, 3> AABBox_T<T>::HalfSize() const noexcept
{
    return (max_ - min_) / 2.0f;
}

template <typename T>
Vector_T<T, 3> AABBox_T<T>::Corner(size_t index) const noexcept
{
    COMMON_ASSERT(index < 8);

    return Vector_T<T, 3>((index & 1UL) ? this->Max().x() : this->Min().x(),
        (index & 2UL) ? this->Max().y() : this->Min().y(),
        (index & 4UL) ? this->Max().z() : this->Min().z());
}

template <typename T>
bool AABBox_T<T>::operator==(AABBox_T<T> const & rhs) const noexcept
{
    return (this->Min() == rhs.Min()) && (this->Max() == rhs.Max());
}

template class AABBox_T<float>;
}