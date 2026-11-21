#include "D3D12FrameBuffer.h"
#include "D3D12RenderEngine.h"
#include "D3D12Texture.h"
#include <base/ZEngine.h>
#include <render/RenderFactory.h>
#include <math/color.h>

namespace RenderWorker
{

D3D12FrameBuffer::D3D12FrameBuffer() = default;
D3D12FrameBuffer::~D3D12FrameBuffer() = default;

void D3D12FrameBuffer::OnBind()
{
	views_dirty_ = false;
}

void D3D12FrameBuffer::OnUnbind()
{
}

void D3D12FrameBuffer::Clear(uint32_t flags, Color const& clr, float depth, int32_t stencil)
{
	auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	re.EndRenderPass();
	bool const clear_color = (flags & CBM_Color) != 0;
	bool const clear_depth = (flags & CBM_Depth) != 0;
	bool const clear_stencil = (flags & CBM_Stencil) != 0;
	re.EnsureRenderPass(clear_color, &clr, clear_depth, depth, clear_stencil, stencil);
	re.EndRenderPass();
}

void D3D12FrameBuffer::Discard([[maybe_unused]] uint32_t flags)
{
}

} // namespace RenderWorker
