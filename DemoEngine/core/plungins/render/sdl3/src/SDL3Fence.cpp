#include "SDL3Fence.h"

namespace RenderWorker
{

uint64_t SDL3Fence::Signal([[maybe_unused]] FenceType ft)
{
	return ++last_signal_;
}

void SDL3Fence::Wait([[maybe_unused]] uint64_t id)
{
}

bool SDL3Fence::Completed([[maybe_unused]] uint64_t id)
{
	return true;
}

} // namespace RenderWorker
