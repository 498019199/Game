#pragma once

#include <common/common.h>
#include <render/ElementFormat.h>

namespace RenderWorker
{

enum BufferUsage
{
    BU_Static,
    BU_Dynamic
};

enum BufferAccess
{
    BA_Read_Only,
    BA_Write_Only,
    BA_Read_Write,
    BA_Write_No_Overwrite
};

class GraphicsBuffer
{
public:
    class Mapper final
    {
        friend class GraphicsBuffer;
    public:
        Mapper(GraphicsBuffer& buffer, BufferAccess ba)
            : buffer_(buffer)
        {
            data_ = buffer_.Map(ba);
        }
        ~Mapper()
        {
            buffer_.Unmap();
        }

        template <typename T>
        const T* Pointer() const
        {
            return static_cast<T*>(data_);
        }
        template <typename T>
        T* Pointer()
        {
            return static_cast<T*>(data_);
        }

    private:
        GraphicsBuffer& buffer_;
        void* data_;
    };

public:
	GraphicsBuffer(BufferUsage usage, uint32_t access_hint, uint32_t size_in_byte, uint32_t structure_byte_stride);
	virtual ~GraphicsBuffer() noexcept;
    
    uint32_t Size() const noexcept
    {
        return size_in_byte_;
    }

    BufferUsage Usage() const noexcept
    {
        return usage_;
    }

    uint32_t AccessHint() const noexcept
    {
        return access_hint_;
    }

    uint32_t StructureByteStride() const noexcept
    {
        return structure_byte_stride_;
    }
    
    virtual void CopyToBuffer(GraphicsBuffer& target) = 0;
    virtual void CopyToSubBuffer(GraphicsBuffer& target, uint32_t dst_offset, uint32_t src_offset, uint32_t size) = 0;

    virtual void CreateHWResource(void const * init_data) = 0;
    virtual void DeleteHWResource() = 0;
    virtual bool HWResourceReady() const = 0;
    
    virtual void UpdateSubresource(uint32_t offset, uint32_t size, void const * data) = 0;

private:
    virtual void* Map(BufferAccess ba) = 0;
    virtual void Unmap() = 0;

protected:
    BufferUsage usage_;
    uint32_t access_hint_;

    uint32_t size_in_byte_;
    uint32_t structure_byte_stride_;
};

using GraphicsBufferPtr = std::shared_ptr<GraphicsBuffer>;
}