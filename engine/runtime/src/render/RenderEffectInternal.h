#pragma once

// Internal helpers shared by RenderEffect.cpp and RenderVariable.cpp.
// Not part of the public API.

#include <render/RenderDeviceCaps.h>
#include <render/RenderEffect.h>
#include <render/RenderStateObject.h>

#include <common/ResIdentifier.h>

#include <memory>
#include <ostream>
#include <string>
#include <string_view>

namespace RenderWorker
{
enum class EffectParameterType;
namespace detail
{
inline constexpr uint32_t EffectDescCacheMagic = 0x31584645; // EFX1
uint64_t EffectSourceFingerprint(std::string_view source);
std::string ReadEffectShaderSource(EffectDesc const& desc);
#if ZENGINE_IS_DEV_PLATFORM
RenderEffectDataType EffectType(EffectParameterType type);
std::unique_ptr<RenderVariable> CreateVariable(RenderEffect const& effect, EffectParameterDesc const& parameter);
std::string_view TypeNameFromCode(RenderEffectDataType type);

void StreamOutVariable(std::ostream& os, RenderVariable const& var);
#endif

std::unique_ptr<RenderVariable> StreamInVariable(
	RenderEffect const& effect, CommonWorker::ResIdentifier& res, RenderEffectDataType type, uint32_t array_size);
} // namespace detail
} // namespace RenderWorker
