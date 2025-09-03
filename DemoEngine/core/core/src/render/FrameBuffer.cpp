#include <render/FrameBuffer.h>

namespace RenderWorker
{

FrameBuffer::FrameBuffer()
{
    
}

FrameBuffer::~FrameBuffer() noexcept  = default;

uint32_t FrameBuffer::Left() const
{
	return left_;
}

uint32_t FrameBuffer::Top() const
{
    return top_;
}

uint32_t FrameBuffer::Width() const
{
    return width_;
}

uint32_t FrameBuffer::Height() const
{
    return height_;
}
    
}