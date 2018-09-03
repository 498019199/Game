#ifndef _STX_MATH_BOUND_
#define _STX_MATH_BOUND_
#pragma once

// ��ֲ klayGE 2018��9��2�� zhangbei
template <typename T>
class Bound_T
{
public:
	virtual ~Bound_T() noexcept
	{
	}

	virtual bool IsEmpty() const noexcept = 0;

	virtual bool VecInBound(Vector_T<T, 3> const & v) const noexcept = 0;
	virtual T MaxRadiusSq() const noexcept = 0;
};
#endif// _STX_MATH_BOUND_
