#include "D3D12Query.h"

namespace RenderWorker
{

void D3D12ConditionalRender::Begin() {}
void D3D12ConditionalRender::End() {}
void D3D12ConditionalRender::BeginConditionalRender() {}
void D3D12ConditionalRender::EndConditionalRender() {}
bool D3D12ConditionalRender::AnySamplesPassed() { return true; }

} // namespace RenderWorker
