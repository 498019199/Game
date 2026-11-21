#include "D3D12RenderStateObject.h"
#include "D3D12RenderEngine.h"
#include <base/ZEngine.h>
#include <render/RenderFactory.h>
#include <algorithm>

namespace RenderWorker
{

D3D12RenderStateObject::D3D12RenderStateObject(const RasterizerStateDesc& rs_desc, const DepthStencilStateDesc& dss_desc,
	const BlendStateDesc& bs_desc)
	: RenderStateObject(rs_desc, dss_desc, bs_desc)
{
}

void D3D12RenderStateObject::Active()
{
}

D3D12SamplerStateObject::D3D12SamplerStateObject(SamplerStateDesc const& desc)
	: SamplerStateObject(desc)
{
	auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	d3d12::Device* device = re.Device();
	if (!device)
	{
		LogError() << "[D3D12] MakeSamplerStateObject before GPU device is ready" << std::endl;
		return;
	}

	uint32_t const filter_bits = static_cast<uint32_t>(desc.filter);
	bool const aniso = (filter_bits & TFOE_Anisotropic) != 0;
	bool const compare = (filter_bits & TFOE_Comparison) != 0;

	d3d12::SamplerCreateInfo info{};
	info.address_mode_u = D3D12Mapping::Mapping(desc.addr_mode_u);
	info.address_mode_v = D3D12Mapping::Mapping(desc.addr_mode_v);
	info.address_mode_w = D3D12Mapping::Mapping(desc.addr_mode_w);
	info.mip_lod_bias = desc.mip_map_lod_bias;
	info.min_lod = desc.min_lod;
	info.max_lod = desc.max_lod;
	info.enable_compare = compare;
	info.compare_op = compare ? D3D12Mapping::Mapping(desc.cmp_func) : d3d12::COMPAREOP_NEVER;

	// Anisotropic filters require linear minification, magnification and mip filtering.
	// TFO_Anisotropic alone has no mip-linear bit; mapping mip→NEAREST yields an
	// unrecognized Filter and removes the device (CREATE_SAMPLER_INVALID).
	if (aniso)
	{
		info.min_filter = d3d12::FILTER_LINEAR;
		info.mag_filter = d3d12::FILTER_LINEAR;
		info.mipmap_mode = d3d12::SAMPLERMIPMAPMODE_LINEAR;
		info.enable_anisotropy = true;
		uint32_t aniso_level = std::max<uint32_t>(2u, desc.max_anisotropy);
		aniso_level = std::min<uint32_t>(16u, aniso_level);
		info.max_anisotropy = static_cast<float>(aniso_level);
	}
	else
	{
		info.min_filter = D3D12Mapping::MappingMinMagFilter(desc.filter, false);
		info.mag_filter = D3D12Mapping::MappingMinMagFilter(desc.filter, true);
		info.mipmap_mode = D3D12Mapping::MappingMipMode(desc.filter);
		info.enable_anisotropy = false;
		info.max_anisotropy = 1.0f;
	}

	sampler_ = d3d12::CreateSampler(device, &info);
	if (!sampler_)
	{
		LogError() << "[D3D12] d3d12::CreateSampler failed: " << d3d12::LastError() << std::endl;
	}
	else
	{
		device_ = device;
		device_lifetime_ = re.DeviceLifetime();
	}
}

D3D12SamplerStateObject::~D3D12SamplerStateObject()
{
	if (sampler_)
	{
		if (device_lifetime_ && (device_lifetime_->device == device_))
		{
			d3d12::ReleaseSampler(device_, sampler_);
		}
		sampler_ = nullptr;
	}
	device_lifetime_.reset();
	device_ = nullptr;
}

} // namespace RenderWorker
