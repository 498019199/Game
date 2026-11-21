#include "MetalFrameBuffer.h"
#include "MetalRenderEngine.h"
#include "MetalTexture.h"
#include <base/ZEngine.h>
#include <render/RenderFactory.h>
#include <math/color.h>

namespace RenderWorker
{

MetalFrameBuffer::MetalFrameBuffer() = default;
MetalFrameBuffer::~MetalFrameBuffer() = default;

void MetalFrameBuffer::OnBind()
{
	views_dirty_ = false;
}

void MetalFrameBuffer::OnUnbind()
{
}

void MetalFrameBuffer::Clear(uint32_t flags, Color const& clr, float depth, int32_t stencil)
{
	auto& re = checked_cast<MetalRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	re.EndRenderPass();
	bool const clear_color = (flags & CBM_Color) != 0;
	bool const clear_depth = (flags & CBM_Depth) != 0;
	bool const clear_stencil = (flags & CBM_Stencil) != 0;
	re.EnsureRenderPass(clear_color, &clr, clear_depth, depth, clear_stencil, stencil);
	re.EndRenderPass();
}

void MetalFrameBuffer::Discard([[maybe_unused]] uint32_t flags)
{
}

} // namespace RenderWorker
