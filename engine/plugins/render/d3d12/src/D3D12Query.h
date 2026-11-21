#pragma once

#include <render/Query.h>

namespace RenderWorker
{

class D3D12ConditionalRender final : public ConditionalRender
{
public:
	void Begin() override;
	void End() override;
	void BeginConditionalRender() override;
	void EndConditionalRender() override;
	bool AnySamplesPassed() override;
};

} // namespace RenderWorker
