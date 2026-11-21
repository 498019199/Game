#include "MetalQuery.h"

namespace RenderWorker
{

void MetalConditionalRender::Begin() {}
void MetalConditionalRender::End() {}
void MetalConditionalRender::BeginConditionalRender() {}
void MetalConditionalRender::EndConditionalRender() {}
bool MetalConditionalRender::AnySamplesPassed() { return true; }

} // namespace RenderWorker
