#ifndef ZENGINE_DEV_HELPS_METADATA_UTIL_H
#define ZENGINE_DEV_HELPS_METADATA_UTIL_H

#pragma once
#include <common/JsonDom.h>
#include <base/Context.h>

namespace RenderWorker
{
	float GetFloat(const JsonValue& value);
	int GetInt(const JsonValue& value);
} // namespace RenderWorker
#endif // ZENGINE_DEV_HELPS_METADATA_UTIL_H