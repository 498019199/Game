#include <render/RenderView.h>

namespace RenderWorker
{

ShaderResourceView::ShaderResourceView() = default;
ShaderResourceView::~ShaderResourceView() noexcept = default;

#if ZENGINE_IS_DEV_PLATFORM
void* ShaderResourceView::GetShaderResourceView()
{
    return nullptr;
}
#endif //ZENGINE_IS_DEV_PLATFORM

RenderTargetView::RenderTargetView() = default;
RenderTargetView::~RenderTargetView() noexcept = default;

DepthStencilView::DepthStencilView() = default;
DepthStencilView::~DepthStencilView() noexcept = default;

UnorderedAccessView::UnorderedAccessView() = default;
UnorderedAccessView::~UnorderedAccessView() noexcept = default;
}