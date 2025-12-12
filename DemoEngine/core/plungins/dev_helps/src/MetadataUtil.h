#pragma once
#include <common/JsonDom.h>
#include <base/Context.h>

namespace RenderWorker
{
	float GetFloat(const JsonValue& value);
	int GetInt(const JsonValue& value);
} // namespace RenderWorker
