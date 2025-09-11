#pragma once
#include <common/macro.h>
#include <common/Log.h>
#include <common/instance.h>
#include <common/defer.h>
#include <common/Util.h>
#include <common/ErrorHandling.h>
#include <common/ResIdentifier.h>
#include <common/Hash.h>
#include <common/StringUtil.h>
#include <common/com_ptr.h>

#include <vector>
#include <map>
#include <common/span.h>
#include <string>
#include <string_view>

#include <math/math.h>
namespace RenderWorker
{
using int1 = MathWorker::int1;
using int2 = MathWorker::int2;
using int3 = MathWorker::int3;
using int4 = MathWorker::int4;
using uint1 = MathWorker::uint1;
using uint2 = MathWorker::uint2;
using uint3 = MathWorker::uint3;
using uint4 = MathWorker::uint4;
using float1 = MathWorker::float1;
using float2 = MathWorker::float2;
using float3 = MathWorker::float3;
using float4 = MathWorker::float4;
using quater = MathWorker::quater;
using rotator = MathWorker::rotator;
using float4x4 = MathWorker::float4x4;
using Color = MathWorker::Color;

using ResIdentifier = CommonWorker::ResIdentifier;
using ResIdentifierPtr = CommonWorker::ResIdentifierPtr;
}