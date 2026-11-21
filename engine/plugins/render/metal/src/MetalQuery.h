#pragma once

#include <render/Query.h>

namespace RenderWorker
{

class MetalConditionalRender final : public ConditionalRender
{
public:
	void Begin() override;
	void End() override;
	void BeginConditionalRender() override;
	void EndConditionalRender() override;
	bool AnySamplesPassed() override;
};

} // namespace RenderWorker
