#include <math/matrix.h>
#include <math/math.h>
namespace RenderWorker
{

template <typename T>
Matrix4_T<T>::Matrix4_T(const T* rhs) noexcept
{
	for (Matrix4_T::size_type i = 0; i < row_num; ++i)
	{
		this->m_[i] = Vector_T<T, col_num>(rhs);
		rhs += col_num;
	}
}

template <typename T>
Matrix4_T<T>::Matrix4_T(const Matrix4_T& rhs) noexcept
	:m_(std::move(rhs.m_))
{

}

template <typename T>
Matrix4_T<T>::Matrix4_T(Matrix4_T&& rhs) noexcept
    :m_(std::move(rhs.m_))
{

}

template <typename T>
Matrix4_T<T>::Matrix4_T(T f11, T f12, T f13, T f14
    , T f21, T f22, T f23, T f24
    , T f31, T f32, T f33, T f34
    , T f41, T f42, T f43, T f44) noexcept
{
    m_[0][0] = f11; m_[0][1] = f12; m_[0][2] = f13; m_[0][3] = f14;
    m_[1][0] = f21; m_[1][1] = f22; m_[1][2] = f23; m_[1][3] = f24;
    m_[2][0] = f31; m_[2][1] = f32; m_[2][2] = f33; m_[2][3] = f34;
    m_[3][0] = f41; m_[3][1] = f42; m_[3][2] = f43; m_[3][3] = f44;
}

template <typename T>
Vector_T<T, 4> Matrix4_T<T>::Col(size_t index) const noexcept
{
	Vector_T<T, col_num> tmp;
	for (Matrix4_T::size_type i = 0; i < row_num; ++i)
	{
		tmp[i] = this->m_[i][index];
	}

	return tmp;
}

template <typename T>
const Vector_T<T, 4>& Matrix4_T<T>::Row(size_t index)  const noexcept
{
	return this->m_[index];
}

template <typename T>
void Matrix4_T<T>::Col(size_t index, const Vector_T<T, 4>& rhs) noexcept
{
	for (Matrix4_T::size_type i = 0; i < row_num; ++i)
	{
		this->m_[i][index] = rhs[i];
	}
}

template <typename T>
void Matrix4_T<T>::Row(size_t index, const Vector_T<T, 4>& rhs) noexcept
{
	this->m_[index] = rhs;
}

template <typename T>
Matrix4_T<T> Matrix4_T<T>::operator-() const noexcept
{
	Matrix4_T<T> tmp(*this);
	tmp.m_ = -this->m_;
	return tmp;
}

template <typename T>
Matrix4_T<T> Matrix4_T<T>::operator+() const noexcept
{
	return *this;
}

template <typename T>
Matrix4_T<T>& Matrix4_T<T>::operator=(Matrix4_T&& rhs) noexcept
{
	this->m_ = std::move(rhs.m_);
	return *this;
}

template <typename T>
Matrix4_T<T>& 
Matrix4_T<T>::operator=(const Matrix4_T& rhs) noexcept
{
	if (this != &rhs)
	{
		this->m_ = rhs.m_;
	}

	return *this;
}

template <typename T>
Matrix4_T<T>& 
Matrix4_T<T>::operator/=(value_type rhs) noexcept
{
	return this->operator*=(1/rhs);
}

template <typename T>
Matrix4_T<T>& 
Matrix4_T<T>::operator*=(value_type rhs) noexcept
{
	for (size_type i = 0; i < row_num; ++i)
	{
		this->m_[i] *= rhs;
	}
	return *this;
}

template <typename T>
Matrix4_T<T>& 
Matrix4_T<T>::operator*=(const Matrix4_T& rhs) noexcept
{
	*this = MathWorker::mul(*this, rhs);
	return *this;
}

template <typename T>
Matrix4_T<T>& 
Matrix4_T<T>::operator-=(const Matrix4_T& rhs) noexcept
{
	this->m_ -= rhs.m_;
	return *this;
}

template <typename T>
Matrix4_T<T>& 
Matrix4_T<T>::operator+=(const Matrix4_T<T>& rhs) noexcept
{
	this->m_ += rhs.m_;
	return *this;
}

template <typename T>
const Matrix4_T<T> & 
Matrix4_T<T>::Zero() noexcept
{
    static const Matrix4_T out(0, 0, 0, 0,
                                0, 0, 0, 0,
                                0, 0, 0, 0,
                                0, 0, 0, 0);
    return out;
}

template <typename T>
const Matrix4_T<T> & 
Matrix4_T<T>::Identity() noexcept
{
    static const Matrix4_T out(1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1);
    return out;
}

template <typename T>
bool Matrix4_T<T>::operator==(const Matrix4_T<T>& rhs) const noexcept
{
	return m_ == rhs.m_;
}


// 实例化模板
template class Matrix4_T<float>;
}