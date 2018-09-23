// 2018��9��18����ֲklayGE @zhangbei

#include "../Core/predefine.h"
#include "../Core/Context.h"

#include "../Platform/Window/Window.h"
#include "../MsgInput/MInput.hpp"

#if (defined STX_PLATFORM_WINDOWS_DESKTOP) || (defined STX_PLATFORM_ANDROID)
#if defined STX_PLATFORM_WINDOWS_DESKTOP
MsgInputJoystick::MsgInputJoystick(HANDLE device)
	: device_id_(0xFFFFFFFF),
#elif defined STX_PLATFORM_ANDROID
MsgInputJoystick::MsgInputJoystick()
	:
#endif
		pos_state_(0, 0, 0), rot_state_(0, 0, 0), slider_state_(0, 0)
{
	buttons_state_.fill(false);

#if defined STX_PLATFORM_WINDOWS_DESKTOP
	MsgInputEngine const & mie = *Context::Instance()->GetSubsystem<MsgInputEngine>();

	UINT size = 0;
	if (0 == ::GetRawInputDeviceInfo(device, RIDI_PREPARSEDDATA, nullptr, &size))
	{
		std::vector<uint8_t> buf(size);
		::GetRawInputDeviceInfo(device, RIDI_PREPARSEDDATA, &buf[0], &size);

		PHIDP_PREPARSED_DATA preparsed_data = reinterpret_cast<PHIDP_PREPARSED_DATA>(&buf[0]);

		HIDP_CAPS caps;
		if (HIDP_STATUS_SUCCESS == mie.HidP_GetCaps(preparsed_data, &caps))
		{
			std::vector<HIDP_BUTTON_CAPS> button_caps(caps.NumberInputButtonCaps);

			uint16_t caps_length = caps.NumberInputButtonCaps;
			if (HIDP_STATUS_SUCCESS == mie.HidP_GetButtonCaps(HidP_Input, &button_caps[0], &caps_length, preparsed_data))
			{
				num_buttons_ = std::min<uint32_t>(static_cast<uint32_t>(buttons_[0].size()),
					button_caps[0].Range.UsageMax - button_caps[0].Range.UsageMin + 1);
			}
		}
	}

	size = 0;
	if (0 == ::GetRawInputDeviceInfo(device, RIDI_DEVICEINFO, nullptr, &size))
	{
		std::vector<uint8_t> buf(size);
		::GetRawInputDeviceInfo(device, RIDI_DEVICEINFO, &buf[0], &size);

		RID_DEVICE_INFO* info = reinterpret_cast<RID_DEVICE_INFO*>(&buf[0]);
		device_id_ = info->hid.dwProductId;
	}
#elif defined STX_PLATFORM_ANDROID
	num_buttons_ = 32;
#endif
}
	
const std::wstring& MsgInputJoystick::Name() const
{
	static std::wstring const name(L"MsgInput Joystick");
	return name;
}

#if defined STX_PLATFORM_WINDOWS_DESKTOP
void MsgInputJoystick::OnRawInput(RAWINPUT const & ri)
{
	BOOST_ASSERT(RIM_TYPEHID == ri.header.dwType);

	UINT size = 0;
	if (0 == ::GetRawInputDeviceInfo(ri.header.hDevice, RIDI_DEVICEINFO, nullptr, &size))
	{
		std::vector<uint8_t> buf(size);
		::GetRawInputDeviceInfo(ri.header.hDevice, RIDI_DEVICEINFO, &buf[0], &size);

		RID_DEVICE_INFO* info = reinterpret_cast<RID_DEVICE_INFO*>(&buf[0]);
		if (device_id_ != info->hid.dwProductId)
		{
			return;
		}
	}

	MsgInputEngine const & mie = *Context::Instance()->GetSubsystem<MsgInputEngine>();
	size = 0;
	if (0 == ::GetRawInputDeviceInfo(ri.header.hDevice, RIDI_PREPARSEDDATA, nullptr, &size))
	{
		std::vector<uint8_t> buf(size);
		::GetRawInputDeviceInfo(ri.header.hDevice, RIDI_PREPARSEDDATA, &buf[0], &size);

		PHIDP_PREPARSED_DATA preparsed_data = reinterpret_cast<PHIDP_PREPARSED_DATA>(&buf[0]);

		HIDP_CAPS caps;
		if (HIDP_STATUS_SUCCESS == mie.HidP_GetCaps(preparsed_data, &caps))
		{
			std::vector<HIDP_BUTTON_CAPS> button_caps(caps.NumberInputButtonCaps);

			uint16_t caps_length = caps.NumberInputButtonCaps;
			if (HIDP_STATUS_SUCCESS == mie.HidP_GetButtonCaps(HidP_Input, &button_caps[0], &caps_length, preparsed_data))
			{
				std::vector<HIDP_VALUE_CAPS> value_caps(caps.NumberInputValueCaps);
				caps_length = caps.NumberInputValueCaps;
				if (HIDP_STATUS_SUCCESS == mie.HidP_GetValueCaps(HidP_Input, &value_caps[0], &caps_length, preparsed_data))
				{
					USAGE usage[32];
					ULONG usage_length = num_buttons_;
					if (HIDP_STATUS_SUCCESS == mie.HidP_GetUsages(HidP_Input, button_caps[0].UsagePage,
						0, usage, &usage_length, preparsed_data,
						reinterpret_cast<CHAR*>(const_cast<BYTE*>(ri.data.hid.bRawData)), ri.data.hid.dwSizeHid))
					{
						buttons_state_.fill(false);
						for (uint32_t i = 0; i < usage_length; ++ i)
						{
							buttons_state_[usage[i] - button_caps[0].Range.UsageMin] = true;
						}

						for (uint32_t i = 0; i < caps.NumberInputValueCaps; ++ i)
						{
							long center;
							long shift;
							switch (value_caps[i].BitField)
							{
							case 1:
								center = 0x80;
								shift = 0;
								break;

							case 2:
								center = 0x8000;
								shift = 8;
								break;

							default:
								center = 0;
								shift = 0;
								break;
							}

							ULONG value;
							if (HIDP_STATUS_SUCCESS == mie.HidP_GetUsageValue(HidP_Input, value_caps[i].UsagePage,
								0, value_caps[i].Range.UsageMin, &value, preparsed_data,
								reinterpret_cast<CHAR*>(const_cast<BYTE*>(ri.data.hid.bRawData)), ri.data.hid.dwSizeHid))
							{
								switch (value_caps[i].Range.UsageMin)
								{
								case HID_USAGE_GENERIC_X:
									pos_state_.x() = (static_cast<long>(value) - center) >> shift;
									break;

								case HID_USAGE_GENERIC_Y:
									pos_state_.y() = (static_cast<long>(value) - center) >> shift;
									break;

								case HID_USAGE_GENERIC_Z:
									pos_state_.z() = (static_cast<long>(value) - center) >> shift;
									break;

								case HID_USAGE_GENERIC_RX:
									rot_state_.x() = (static_cast<long>(value) - center) >> shift;
									break;

								case HID_USAGE_GENERIC_RY:
									rot_state_.y() = (static_cast<long>(value) - center) >> shift;
									break;

								case HID_USAGE_GENERIC_RZ:
									rot_state_.z() = (static_cast<long>(value) - center) >> shift;
									break;

								case HID_USAGE_GENERIC_SLIDER:
									slider_state_.x() = (static_cast<long>(value) - center) >> shift;
									break;

								case HID_USAGE_GENERIC_DIAL:
									slider_state_.y() = (static_cast<long>(value) - center) >> shift;
									break;

								default:
									break;
								}
							}
						}
					}
				}
			}
		}
	}
}
#elif defined STX_PLATFORM_ANDROID
void MsgInputJoystick::OnJoystickAxis(uint32_t axis, int32_t value)
{
	// TODO: Is it correct?
	switch (axis)
	{
	case 0:
		pos_state_.x() = value;
		break;

	case 1:
		pos_state_.y() = value;
		break;

	case 2:
		pos_state_.z() = value;
		break;

	case 3:
		rot_state_.x() = value;
		break;

	case 4:
		rot_state_.y() = value;
		break;

	case 5:
		rot_state_.z() = value;
		break;

	case 6:
		slider_state_.x() = value;
		break;

	case 7:
		slider_state_.y() = value;
		break;

	default:
		break;
	}
}

void MsgInputJoystick::OnJoystickButtons(uint32_t buttons)
{
	for (size_t i = 0; i < buttons_state_.size(); ++ i)
	{
		buttons_state_[i] = (buttons & (1UL << i)) ? true : false;
	}
}
#endif

void MsgInputJoystick::UpdateInputs()
{
	pos_ = pos_state_;
	rot_ = rot_state_;
	slider_ = slider_state_;

	index_ = !index_;
	buttons_[index_] = buttons_state_;
}
#endif
