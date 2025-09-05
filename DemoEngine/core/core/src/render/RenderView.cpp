#include <render/RenderView.h>

namespace RenderWorker
{

ShaderResourceView::ShaderResourceView() = default;
ShaderResourceView::~ShaderResourceView() noexcept = default;

RenderTargetView::RenderTargetView() = default;
RenderTargetView::~RenderTargetView() noexcept = default;

DepthStencilView::DepthStencilView() = default;
DepthStencilView::~DepthStencilView() noexcept = default;

UnorderedAccessView::UnorderedAccessView() = default;
UnorderedAccessView::~UnorderedAccessView() noexcept = default;
}