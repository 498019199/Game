#include "MetalFence.h"

namespace RenderWorker
{

uint64_t MetalFence::Signal([[maybe_unused]] FenceType ft)
{
	return ++last_signal_;
}

void MetalFence::Wait([[maybe_unused]] uint64_t id)
{
}

bool MetalFence::Completed([[maybe_unused]] uint64_t id)
{
	return true;
}

} // namespace RenderWorker
