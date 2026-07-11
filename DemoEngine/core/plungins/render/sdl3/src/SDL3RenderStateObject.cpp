#include "SDL3RenderStateObject.h"

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
}

} // namespace RenderWorker
