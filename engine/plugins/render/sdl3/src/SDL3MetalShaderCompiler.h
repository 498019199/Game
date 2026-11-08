#pragma once

#include "SDL3ShaderObject.h"

namespace RenderWorker
{
// Returns MSL and reflection using the same buffer/resource layout consumed by SDL3RenderEngine.
std::string CompileSDL3MetalShader(RenderEffect const& effect, RenderTechnique const& tech,
    RenderPass const& pass, ShaderStage stage, std::string& entry, SDL3ShaderDesc& desc);
}
