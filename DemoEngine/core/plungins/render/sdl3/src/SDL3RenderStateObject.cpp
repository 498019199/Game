#include "SDL3RenderStateObject.h"
#include "SDL3RenderEngine.h"
#include <base/ZEngine.h>
#include <render/RenderFactory.h>
#include <algorithm>

namespace RenderWorker
{

SDL3RenderStateObject::SDL3RenderStateObject(const RasterizerStateDesc& rs_desc, const DepthStencilStateDesc& dss_desc,
	const BlendStateDesc& bs_desc)
	: RenderStateObject(rs_desc, dss_desc, bs_desc)
{
}

void SDL3RenderStateObject::Active()
{
}

SDL3SamplerStateObject::SDL3SamplerStateObject(SamplerStateDesc const& desc)
	: SamplerStateObject(desc)
{
	auto& re = checked_cast<SDL3RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	SDL_GPUDevice* device = re.Device();
	if (!device)
	{
		LogError() << "[SDL3] MakeSamplerStateObject before GPU device is ready" << std::endl;
		return;
	}

	uint32_t const filter_bits = static_cast<uint32_t>(desc.filter);
	bool const aniso = (filter_bits & TFOE_Anisotropic) != 0;
	bool const compare = (filter_bits & TFOE_Comparison) != 0;

	SDL_GPUSamplerCreateInfo info{};
	info.address_mode_u = SDL3Mapping::Mapping(desc.addr_mode_u);
	info.address_mode_v = SDL3Mapping::Mapping(desc.addr_mode_v);
	info.address_mode_w = SDL3Mapping::Mapping(desc.addr_mode_w);
	info.mip_lod_bias = desc.mip_map_lod_bias;
	info.min_lod = desc.min_lod;
	info.max_lod = desc.max_lod;
	info.enable_compare = compare;
	info.compare_op = compare ? SDL3Mapping::Mapping(desc.cmp_func) : SDL_GPU_COMPAREOP_NEVER;

	// D3D12 anisotropic filters must be full LINEAR min/mag/mip (| ANISO bit).
	// TFO_Anisotropic alone has no mip-linear bit; mapping mip→NEAREST yields an
	// unrecognized Filter and removes the device (CREATE_SAMPLER_INVALID).
	if (aniso)
	{
		info.min_filter = SDL_GPU_FILTER_LINEAR;
		info.mag_filter = SDL_GPU_FILTER_LINEAR;
		info.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;
		info.enable_anisotropy = true;
		uint32_t aniso_level = std::max<uint32_t>(2u, desc.max_anisotropy);
		aniso_level = std::min<uint32_t>(16u, aniso_level);
		info.max_anisotropy = static_cast<float>(aniso_level);
	}
	else
	{
		info.min_filter = SDL3Mapping::MappingMinMagFilter(desc.filter, false);
		info.mag_filter = SDL3Mapping::MappingMinMagFilter(desc.filter, true);
		info.mipmap_mode = SDL3Mapping::MappingMipMode(desc.filter);
		info.enable_anisotropy = false;
		info.max_anisotropy = 1.0f;
	}

	sampler_ = SDL_CreateGPUSampler(device, &info);
	if (!sampler_)
	{
		LogError() << "[SDL3] SDL_CreateGPUSampler failed: " << SDL_GetError() << std::endl;
	}
}

SDL3SamplerStateObject::~SDL3SamplerStateObject()
{
	if (sampler_)
	{
		auto& re = checked_cast<SDL3RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		if (re.Device())
		{
			SDL_ReleaseGPUSampler(re.Device(), sampler_);
		}
		sampler_ = nullptr;
	}
}

} // namespace RenderWorker
