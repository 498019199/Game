#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <render/ElementFormat.h>
#include <render/RenderLayout.h>
#include <render/RenderStateObject.h>
#include <common/Log.h>
#include <common/ErrorHandling.h>

namespace RenderWorker
{

class SDL3Mapping final
{
public:
	static SDL_GPUTextureFormat MappingFormat(ElementFormat format);
	static ElementFormat MappingFormat(SDL_GPUTextureFormat fmt);

	static SDL_GPUPrimitiveType Mapping(RenderLayout::topology_type tt);
	static SDL_GPUCullMode Mapping(CullMode mode);
	static SDL_GPUFillMode Mapping(PolygonMode mode);
	static SDL_GPUCompareOp Mapping(CompareFunction func);
	static SDL_GPUBlendFactor Mapping(AlphaBlendFactor factor);
	static SDL_GPUBlendOp Mapping(BlendOperation op);
};

inline void SDL3Check(bool ok, char const* what)
{
	if (!ok)
	{
		LogError() << "[SDL3] " << what << ": " << SDL_GetError() << std::endl;
		TMSG(what);
	}
}

} // namespace RenderWorker
