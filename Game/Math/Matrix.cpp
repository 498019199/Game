#include "Matrix.h"
#include "Math.h"

template <typename Valty>
Matrix4_T<Valty>::Matrix4_T(const Valty* rhs) noexcept
{
	for (int i = 0; i < row_num; ++i)
	{
		this->m[i] = Vector_T<Valty, col_num>(rhs);
		rhs += col_num;
	}
}

template <typename Valty>
Matrix4_T<Valty>::Matrix4_T(const Matrix4_T& rhs) noexcept
	:m(std::move(rhs.m))
{

}

template <typename Valty>
Matrix4_T<Valty>::Matrix4_T(Matrix4_T&& rhs) noexcept
    :m(std::move(rhs.m))
{

}

template <typename Valty>
Matrix4_T<Valty>::Matrix4_T(Valty f11, Valty f12, Valty f13, Valty f14
    , Valty f21, Valty f22, Valty f23, Valty f24
    , Valty f31, Valty f32, Valty f33, Valty f34
    , Valty f41, Valty f42, Valty f43, Valty f44) noexcept
{
    m[0][0] = f11; m[0][1] = f12; m[0][2] = f13; m[0][3] = f14;
    m[1][0] = f21; m[1][1] = f22; m[1][2] = f23; m[1][3] = f24;
    m[2][0] = f31; m[2][1] = f32; m[2][2] = f33; m[2][3] = f34;
    m[3][0] = f41; m[3][1] = f42; m[3][2] = f43; m[3][3] = f44;
}

template <typename Valty>
Vector_T<Valty, 4> Matrix4_T<Valty>::Col(size_t index) const noexcept
{
	Vector_T<Valty, col_num> tmp;
	for (int i = 0; i < row_num; ++i)
	{
		tmp[i] = this->m[i][index];
	}

	return tmp;
}

template <typename Valty>
const Vector_T<Valty, 4>& Matrix4_T<Valty>::Row(size_t index)  const noexcept
{
	return this->m[index];
}

template <typename Valty>
void Matrix4_T<Valty>::Col(size_t index, const Vector_T<Valty, 4>& rhs) noexcept
{
	for (int i = 0; i < row_num; ++i)
	{
		this->m[i][index] = rhs[i];
	}
}

template <typename Valty>
void Matrix4_T<Valty>::Row(size_t index, const Vector_T<Valty, 4>& rhs) noexcept
{
	this->m[index] = rhs;
}

template <typename Valty>
Matrix4_T<Valty> Matrix4_T<Valty>::operator-() const noexcept
{
	Matrix4_T<Valty> tmp(*this);
	tmp.m = -this->m;
	return tmp;
}

template <typename Valty>
Matrix4_T<Valty> Matrix4_T<Valty>::operator+() const noexcept
{
	return *this;
}

template <typename Valty>
Matrix4_T<Valty>& Matrix4_T<Valty>::operator=(Matrix4_T&& rhs) noexcept
{
	this->m = std::move(rhs.m);
	return *this;
}

template <typename Valty>
Matrix4_T<Valty>& Matrix4_T<Valty>::operator=(const Matrix4_T& rhs) noexcept
{
	if (this != &rhs)
	{
		this->m = rhs.m;
	}

	return *this;
}

template <typename Valty>
Matrix4_T<Valty>& Matrix4_T<Valty>::operator/=(value_type rhs) noexcept
{
	return this->operator*=(MathLib::RecipSqrt(rhs));
}

template <typename Valty>
Matrix4_T<Valty>& Matrix4_T<Valty>::operator*=(value_type rhs) noexcept
{
	for (size_type i = 0; i < row_num; ++i)
	{
		this->m[i] *= rhs;
	}
	return *this;
}

template <typename Valty>
Matrix4_T<Valty>& Matrix4_T<Valty>::operator*=(const Matrix4_T& rhs) noexcept
{
	*this = MathLib::Mul(*this, rhs);
	return *this;
}

template <typename Valty>
Matrix4_T<Valty>& Matrix4_T<Valty>::operator-=(const Matrix4_T& rhs) noexcept
{
	this->m -= rhs.m;
	return *this;
}

template <typename Valty>
Matrix4_T<Valty>& Matrix4_T<Valty>::operator+=(const Matrix4_T<Valty>& rhs) noexcept
{
	this->m += rhs.m;
	return *this;
}


template <typename Valty>
const Matrix4_T<Valty> & Matrix4_T<Valty>::Zero() noexcept
{
    static const Matrix4_T out(0, 0, 0, 0,
                                             0, 0, 0, 0,
                                             0, 0, 0, 0,
                                             0, 0, 0, 0);
    return out;
}

template <typename Valty>
const Matrix4_T<Valty> & Matrix4_T<Valty>::Identity() noexcept
{
    static const Matrix4_T out(1, 0, 0, 0,
                                             0, 1, 0, 0,
                                             0, 0, 1, 0,
                                             0, 0, 0, 1);
    return out;
}

template <typename Valty>
bool Matrix4_T<Valty>::operator==(const Matrix4_T<Valty>& rhs) const noexcept
{
	return m == rhs.m;
	// 这里类型比较的是Vector_T<Vector_T<float,4>,4>，实际调用的是Vector_T operator==()
}

// 实例化模板
template class Matrix4_T<float>;