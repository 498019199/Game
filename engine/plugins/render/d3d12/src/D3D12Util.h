#pragma once

#include "D3D12Gpu.h"
#include <render/ElementFormat.h>
#include <render/RenderLayout.h>
#include <render/RenderStateObject.h>
#include <common/Log.h>
#include <common/ErrorHandling.h>

namespace RenderWorker
{

class D3D12Mapping final
{
public:
	static d3d12::TextureFormat MappingFormat(ElementFormat format);
	static ElementFormat MappingFormat(d3d12::TextureFormat fmt);

	static d3d12::PrimitiveType Mapping(RenderLayout::topology_type tt);
	static d3d12::CullMode Mapping(CullMode mode);
	static d3d12::FillMode Mapping(PolygonMode mode);
	static d3d12::CompareOp Mapping(CompareFunction func);
	static d3d12::BlendFactor Mapping(AlphaBlendFactor factor);
	static d3d12::BlendOp Mapping(BlendOperation op);
	static d3d12::Filter MappingMinMagFilter(TexFilterOp filter, bool mag);
	static d3d12::SamplerMipmapMode MappingMipMode(TexFilterOp filter);
	static d3d12::SamplerAddressMode Mapping(TexAddressingMode mode);
};

inline void D3D12Check(bool ok, char const* what)
{
	if (!ok)
	{
		LogError() << "[D3D12] " << what << ": " << d3d12::LastError() << std::endl;
		TMSG(what);
	}
}

} // namespace RenderWorker
