#pragma once

#include <render/RenderStateObject.h>
#include "SDL3Util.h"

namespace RenderWorker
{

class SDL3RenderStateObject final : public RenderStateObject
{
public:
	SDL3RenderStateObject(const RasterizerStateDesc& rs_desc, const DepthStencilStateDesc& dss_desc,
		const BlendStateDesc& bs_desc);
	void Active() override;
};

class SDL3SamplerStateObject final : public SamplerStateObject
{
public:
	explicit SDL3SamplerStateObject(SamplerStateDesc const& desc);
	~SDL3SamplerStateObject() override;

	SDL_GPUSampler* GpuSampler() const noexcept
	{
		return sampler_;
	}

private:
	SDL_GPUSampler* sampler_{nullptr};
};

} // namespace RenderWorker
