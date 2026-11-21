#pragma once

#include "MetalShaderObject.h"

namespace RenderWorker
{
// Returns MSL and reflection using the same buffer/resource layout consumed by MetalRenderEngine.
std::string CompileMetalShader(RenderEffect const& effect, RenderTechnique const& tech,
    RenderPass const& pass, ShaderStage stage, std::string& entry, MetalShaderDesc& desc);
}
