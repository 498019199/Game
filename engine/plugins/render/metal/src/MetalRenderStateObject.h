#pragma once

#include <render/RenderStateObject.h>
#include "MetalUtil.h"

namespace RenderWorker
{

struct MetalDeviceLifetime;

class MetalRenderStateObject final : public RenderStateObject
{
public:
	MetalRenderStateObject(const RasterizerStateDesc& rs_desc, const DepthStencilStateDesc& dss_desc,
		const BlendStateDesc& bs_desc);
	void Active() override;
};

class MetalSamplerStateObject final : public SamplerStateObject
{
public:
	explicit MetalSamplerStateObject(SamplerStateDesc const& desc);
	~MetalSamplerStateObject() override;

	metal::Sampler* GpuSampler() const noexcept
	{
		return sampler_;
	}

private:
	std::shared_ptr<MetalDeviceLifetime> device_lifetime_;
	metal::Device* device_{nullptr};
	metal::Sampler* sampler_{nullptr};
};

} // namespace RenderWorker
