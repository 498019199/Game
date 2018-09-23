// 2018Äê9ÔÂ18ÈÕÒÆÖ²klayGE @zhangbei

#include "../Core/predefine.h"
#include "../Core/Context.h"

#include "../Platform/Window/Window.h"
#include "../MsgInput/MInput.hpp"

MsgInputTouch::MsgInputTouch()
	: wheel_delta_state_(0)
{
	touch_down_state_.fill(false);
}

const std::wstring& MsgInputTouch::Name() const
{
	static std::wstring const name(L"MsgInput Touch");
	return name;
}

void MsgInputTouch::OnPointerDown(int2 const & pt, uint32_t id)
{
	if (id > 0)
	{
		--id;
		touch_coord_state_[id] = this->AdjustPoint(pt);
		touch_down_state_[id] = true;
	}
}

void MsgInputTouch::OnPointerUp(int2 const & pt, uint32_t id)
{
	if (id > 0)
	{
		--id;
		touch_coord_state_[id] = this->AdjustPoint(pt);
		touch_down_state_[id] = false;
	}
}

void MsgInputTouch::OnPointerUpdate(int2 const & pt, uint32_t id, bool down)
{
	if (id > 0)
	{
		--id;
		touch_coord_state_[id] = this->AdjustPoint(pt);
		touch_down_state_[id] = down;
	}
}

void MsgInputTouch::OnPointerWheel(int2 const & pt, uint32_t id, int32_t wheel_delta)
{
	if (id > 0)
	{
		--id;
		touch_coord_state_[id] = this->AdjustPoint(pt);
	}
	wheel_delta_state_ += wheel_delta;
}

void MsgInputTouch::UpdateInputs()
{
	index_ = !index_;
	touch_coords_[index_] = touch_coord_state_;
	touch_downs_[index_] = touch_down_state_;
	wheel_delta_ = wheel_delta_state_;
	num_available_touch_ = 0;
	for (auto const & tds : touch_down_state_)
	{
		num_available_touch_ += tds;
	}

	wheel_delta_state_ = 0;
	gesture_ = TS_None;
	action_param_->move_vec = float2(0.0f, 0.0f);
	curr_gesture_(static_cast<float>(timer_.Elapsed()));
	timer_.ReStart();
}

int2 MsgInputTouch::AdjustPoint(int2 const & pt) const
{
	return pt;
}
