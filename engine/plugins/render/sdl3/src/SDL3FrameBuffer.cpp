#include "SDL3FrameBuffer.h"
#include "SDL3RenderEngine.h"
#include "SDL3Texture.h"
#include <base/ZEngine.h>
#include <render/RenderFactory.h>
#include <math/color.h>

namespace RenderWorker
{

SDL3FrameBuffer::SDL3FrameBuffer() = default;
SDL3FrameBuffer::~SDL3FrameBuffer() = default;

void SDL3FrameBuffer::OnBind()
{
	views_dirty_ = false;
}

void SDL3FrameBuffer::OnUnbind()
{
}

void SDL3FrameBuffer::Clear(uint32_t flags, Color const& clr, float depth, int32_t stencil)
{
	auto& re = checked_cast<SDL3RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	re.EndRenderPass();
	bool const clear_color = (flags & CBM_Color) != 0;
	bool const clear_depth = (flags & CBM_Depth) != 0;
	bool const clear_stencil = (flags & CBM_Stencil) != 0;
	re.EnsureRenderPass(clear_color, &clr, clear_depth, depth, clear_stencil, stencil);
	re.EndRenderPass();
}

void SDL3FrameBuffer::Discard([[maybe_unused]] uint32_t flags)
{
}

} // namespace RenderWorker
