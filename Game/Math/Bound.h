#ifndef _STX_MATH_BOUND_
#define _STX_MATH_BOUND_
#pragma once

// ÒÆÖ² klayGE 2018Äê9ÔÂ2ÈÕ zhangbei
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
