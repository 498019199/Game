#pragma once

#include <render/Fence.h>

namespace RenderWorker
{

class SDL3Fence final : public Fence
{
public:
	uint64_t Signal(FenceType ft) override;
	void Wait(uint64_t id) override;
	bool Completed(uint64_t id) override;

private:
	uint64_t last_signal_{0};
};

} // namespace RenderWorker
