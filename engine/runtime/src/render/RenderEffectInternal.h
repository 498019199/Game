#pragma once

// Internal helpers shared by RenderEffect.cpp and RenderVariable.cpp.
// Not part of the public API.

#include <render/RenderDeviceCaps.h>
#include <render/RenderEffect.h>
#include <render/RenderStateObject.h>

#include <common/ResIdentifier.h>
#include <common/XMLDom.h>

#include <memory>
#include <ostream>
#include <string>
#include <string_view>

namespace RenderWorker
{
namespace detail
{
#if ZENGINE_IS_DEV_PLATFORM
RenderEffectDataType TypeFromName(std::string_view name);
std::string_view TypeNameFromCode(RenderEffectDataType type);

ShadeMode ShadeModeFromName(std::string_view name);
CompareFunction CompareFunctionFromName(std::string_view name);
CullMode CullModeFromName(std::string_view name);
PolygonMode PolygonModeFromName(std::string_view name);
AlphaBlendFactor AlphaBlendFactorFromName(std::string_view name);
BlendOperation BlendOperationFromName(std::string_view name);
StencilOperation StencilOperationFromName(std::string_view name);
TexFilterOp TexFilterOpFromName(std::string_view name);
TexAddressingMode TexAddressingModeFromName(std::string_view name);
LogicOperation LogicOperationFromName(std::string_view name);

int RetrieveIndex(CommonWorker::XMLNode const& node);
std::string RetrieveProfile(CommonWorker::XMLNode const& node);
std::string RetrieveFuncName(CommonWorker::XMLNode const& node);

void LoadVersion(CommonWorker::XMLNode const& node, ShaderModel& ver);

std::unique_ptr<RenderVariable> LoadVariable(
	RenderEffect const& effect, CommonWorker::XMLNode const& node, RenderEffectDataType type, uint32_t array_size);
void StreamOutVariable(std::ostream& os, RenderVariable const& var);
#endif

std::unique_ptr<RenderVariable> StreamInVariable(
	RenderEffect const& effect, CommonWorker::ResIdentifier& res, RenderEffectDataType type, uint32_t array_size);
} // namespace detail
} // namespace RenderWorker
