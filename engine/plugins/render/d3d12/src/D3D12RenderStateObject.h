#pragma once

#include <render/RenderStateObject.h>
#include "D3D12Util.h"

namespace RenderWorker
{

struct D3D12DeviceLifetime;

class D3D12RenderStateObject final : public RenderStateObject
{
public:
	D3D12RenderStateObject(const RasterizerStateDesc& rs_desc, const DepthStencilStateDesc& dss_desc,
		const BlendStateDesc& bs_desc);
	void Active() override;
};

class D3D12SamplerStateObject final : public SamplerStateObject
{
public:
	explicit D3D12SamplerStateObject(SamplerStateDesc const& desc);
	~D3D12SamplerStateObject() override;

	d3d12::Sampler* GpuSampler() const noexcept
	{
		return sampler_;
	}

private:
	std::shared_ptr<D3D12DeviceLifetime> device_lifetime_;
	d3d12::Device* device_{nullptr};
	d3d12::Sampler* sampler_{nullptr};
};

} // namespace RenderWorker
