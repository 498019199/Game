#include <render/GraphicsBuffer.h>

namespace RenderWorker
{

GraphicsBuffer::GraphicsBuffer(BufferUsage usage, uint32_t access_hint, uint32_t size_in_byte, uint32_t structure_byte_stride)
    :usage_(usage), access_hint_(access_hint), size_in_byte_(size_in_byte), structure_byte_stride_(structure_byte_stride)
{
    
}

GraphicsBuffer::~GraphicsBuffer() noexcept = default;

}
