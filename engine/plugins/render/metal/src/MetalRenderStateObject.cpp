#include "MetalRenderStateObject.h"
#include "MetalRenderEngine.h"
#include <base/ZEngine.h>
#include <render/RenderFactory.h>
#include <algorithm>

namespace RenderWorker
{

MetalRenderStateObject::MetalRenderStateObject(const RasterizerStateDesc& rs_desc, const DepthStencilStateDesc& dss_desc,
	const BlendStateDesc& bs_desc)
	: RenderStateObject(rs_desc, dss_desc, bs_desc)
{
}

void MetalRenderStateObject::Active()
{
}

MetalSamplerStateObject::MetalSamplerStateObject(SamplerStateDesc const& desc)
	: SamplerStateObject(desc)
{
	auto& re = checked_cast<MetalRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	metal::Device* device = re.Device();
	if (!device)
	{
		LogError() << "[Metal] MakeSamplerStateObject before GPU device is ready" << std::endl;
		return;
	}

	uint32_t const filter_bits = static_cast<uint32_t>(desc.filter);
	bool const aniso = (filter_bits & TFOE_Anisotropic) != 0;
	bool const compare = (filter_bits & TFOE_Comparison) != 0;

	metal::SamplerCreateInfo info{};
	info.address_mode_u = MetalMapping::Mapping(desc.addr_mode_u);
	info.address_mode_v = MetalMapping::Mapping(desc.addr_mode_v);
	info.address_mode_w = MetalMapping::Mapping(desc.addr_mode_w);
	info.mip_lod_bias = desc.mip_map_lod_bias;
	info.min_lod = desc.min_lod;
	info.max_lod = desc.max_lod;
	info.enable_compare = compare;
	info.compare_op = compare ? MetalMapping::Mapping(desc.cmp_func) : metal::COMPAREOP_NEVER;

	// Use linear filtering for anisotropic sampling.
	// TFO_Anisotropic alone has no mip-linear bit; mapping mip→NEAREST yields an
	// unrecognized Filter and removes the device (CREATE_SAMPLER_INVALID).
	if (aniso)
	{
		info.min_filter = metal::FILTER_LINEAR;
		info.mag_filter = metal::FILTER_LINEAR;
		info.mipmap_mode = metal::SAMPLERMIPMAPMODE_LINEAR;
		info.enable_anisotropy = true;
		uint32_t aniso_level = std::max<uint32_t>(2u, desc.max_anisotropy);
		aniso_level = std::min<uint32_t>(16u, aniso_level);
		info.max_anisotropy = static_cast<float>(aniso_level);
	}
	else
	{
		info.min_filter = MetalMapping::MappingMinMagFilter(desc.filter, false);
		info.mag_filter = MetalMapping::MappingMinMagFilter(desc.filter, true);
		info.mipmap_mode = MetalMapping::MappingMipMode(desc.filter);
		info.enable_anisotropy = false;
		info.max_anisotropy = 1.0f;
	}

	sampler_ = metal::CreateSampler(device, &info);
	if (!sampler_)
	{
		LogError() << "[Metal] metal::CreateSampler failed: " << metal::LastError() << std::endl;
	}
	else
	{
		device_ = device;
		device_lifetime_ = re.DeviceLifetime();
	}
}

MetalSamplerStateObject::~MetalSamplerStateObject()
{
	if (sampler_)
	{
		if (device_lifetime_ && (device_lifetime_->device == device_))
		{
			metal::ReleaseSampler(device_, sampler_);
		}
		sampler_ = nullptr;
	}
	device_lifetime_.reset();
	device_ = nullptr;
}

} // namespace RenderWorker
