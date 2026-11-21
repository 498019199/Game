#pragma once

#include "MetalGpu.h"
#include <render/ElementFormat.h>
#include <render/RenderLayout.h>
#include <render/RenderStateObject.h>
#include <common/Log.h>
#include <common/ErrorHandling.h>

namespace RenderWorker
{

class MetalMapping final
{
public:
	static metal::TextureFormat MappingFormat(ElementFormat format);
	static ElementFormat MappingFormat(metal::TextureFormat fmt);

	static metal::PrimitiveType Mapping(RenderLayout::topology_type tt);
	static metal::CullMode Mapping(CullMode mode);
	static metal::FillMode Mapping(PolygonMode mode);
	static metal::CompareOp Mapping(CompareFunction func);
	static metal::BlendFactor Mapping(AlphaBlendFactor factor);
	static metal::BlendOp Mapping(BlendOperation op);
	static metal::Filter MappingMinMagFilter(TexFilterOp filter, bool mag);
	static metal::SamplerMipmapMode MappingMipMode(TexFilterOp filter);
	static metal::SamplerAddressMode Mapping(TexAddressingMode mode);
};

inline void MetalCheck(bool ok, char const* what)
{
	if (!ok)
	{
		LogError() << "[Metal] " << what << ": " << metal::LastError() << std::endl;
		TMSG(what);
	}
}

} // namespace RenderWorker
