#include "MetadataUtil.h"

namespace RenderWorker
{
	float GetFloat(const JsonValue& value)
	{
		switch (value.Type())
		{
		case JsonValueType::Float:
			return value.ValueFloat();
		case JsonValueType::Int:
			return static_cast<float>(value.ValueInt());
		case JsonValueType::UInt:
			return static_cast<float>(value.ValueUInt());

		default:
			ZENGINE_UNREACHABLE("Invalid value type.");
		}
	}

	int GetInt(const JsonValue& value)
	{
		switch (value.Type())
		{
		case JsonValueType::Int:
			return value.ValueInt();
		case JsonValueType::UInt:
			return static_cast<int>(value.ValueUInt());

		default:
			ZENGINE_UNREACHABLE("Invalid value type.");
		}
	}
} // namespace KlayGE
