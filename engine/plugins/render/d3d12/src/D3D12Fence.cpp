#include "D3D12Fence.h"

namespace RenderWorker
{

uint64_t D3D12Fence::Signal([[maybe_unused]] FenceType ft)
{
	return ++last_signal_;
}

void D3D12Fence::Wait([[maybe_unused]] uint64_t id)
{
}

bool D3D12Fence::Completed([[maybe_unused]] uint64_t id)
{
	return true;
}

} // namespace RenderWorker
