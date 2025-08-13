#include <base/Context.h>

#include "D3D11Util.h"
#include "D3D11GraphicsBuffer.h"
#include "D3D11RenderEngine.h"

namespace RenderWorker
{
using namespace CommonWorker;

D3D11GraphicsBuffer::D3D11GraphicsBuffer(BufferUsage usage, uint32_t access_hint, uint32_t bind_flags,
            uint32_t size_in_byte, uint32_t structure_byte_stride)
            :GraphicsBuffer(usage, access_hint, size_in_byte, structure_byte_stride), bind_flags_(bind_flags)
{
    const auto& d3d11_re = checked_cast<const D3D11RenderEngine&>(Context::Instance().RenderEngineInstance());
    d3d_device_ = d3d11_re.D3DDevice();
    d3d_imm_ctx_ = d3d11_re.D3DDeviceImmContext();
}

void D3D11GraphicsBuffer::CopyToBuffer(GraphicsBuffer& target)
{
    CopyToSubBuffer(target, 0, 0, size_in_byte_);
}

void D3D11GraphicsBuffer::CopyToSubBuffer(GraphicsBuffer& target, uint32_t dst_offset, uint32_t src_offset, uint32_t size)
{
    COMMON_ASSERT(src_offset + size <= this->Size());
    COMMON_ASSERT(dst_offset + size <= target.Size());

    auto& d3d_gb = checked_cast<D3D11GraphicsBuffer&>(target);
    if ((src_offset == 0) && (dst_offset == 0) && (size == this->Size()) && (size == target.Size()))
    {
        d3d_imm_ctx_->CopyResource(d3d_gb.D3DBuffer(), d3d_buffer_.get());
    }
    else
    {
        D3D11_BOX box;
        box.left = src_offset;
        box.right = src_offset + size;
        box.front = 0;
        box.top = 0;
        box.bottom = 1;
        box.back = 1;
        d3d_imm_ctx_->CopySubresourceRegion(d3d_gb.D3DBuffer(), 0, dst_offset, 0, 0, d3d_buffer_.get(), 0, &box);
    }
}

void D3D11GraphicsBuffer::GetD3DFlags(D3D11_USAGE& usage, UINT& cpu_access_flags, UINT& bind_flags, UINT& misc_flags)
{
    if (access_hint_ & EAH_Immutable)
    {
        usage = D3D11_USAGE_IMMUTABLE;
    }
    else
    {
        if ((EAH_CPU_Write == access_hint_) || ((EAH_CPU_Write | EAH_GPU_Read) == access_hint_))
        {
            usage = D3D11_USAGE_DYNAMIC;
        }
        else
        {
            if (!(access_hint_ & EAH_CPU_Read) && !(access_hint_ & EAH_CPU_Write))
            {
                usage = D3D11_USAGE_DEFAULT;
            }
            else
            {
                usage = D3D11_USAGE_STAGING;
            }
        }
    }

    cpu_access_flags = 0;
    if (access_hint_ & EAH_CPU_Read)
    {
        cpu_access_flags |= D3D11_CPU_ACCESS_READ;
    }
    if (access_hint_ & EAH_CPU_Write)
    {
        cpu_access_flags |= D3D11_CPU_ACCESS_WRITE;
    }

    if (D3D11_USAGE_STAGING == usage)
    {
        bind_flags = 0;
    }
    else
    {
        bind_flags = bind_flags_;
    }
    if (bind_flags != D3D11_BIND_CONSTANT_BUFFER)
    {
        if ((access_hint_ & EAH_GPU_Read) && !(access_hint_ & EAH_CPU_Write))
        {
            bind_flags |= D3D11_BIND_SHADER_RESOURCE;
        }
        if (access_hint_ & EAH_GPU_Write)
        {
            if (!((access_hint_ & EAH_GPU_Structured) || (access_hint_ & EAH_GPU_Unordered)))
            {
                bind_flags |= D3D11_BIND_STREAM_OUTPUT;
            }
        }
        if (access_hint_ & EAH_GPU_Unordered)
        {
            bind_flags |= D3D11_BIND_UNORDERED_ACCESS;
        }
    }

    misc_flags = 0;
    if (access_hint_ & EAH_GPU_Unordered)
    {
        misc_flags |= (access_hint_ & EAH_GPU_Structured)
            ? D3D11_RESOURCE_MISC_BUFFER_STRUCTURED : D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
    }
    if (access_hint_ & EAH_DrawIndirectArgs)
    {
        misc_flags |= D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
    }
}

void D3D11GraphicsBuffer::CreateHWResource(void const * init_data)
{
    D3D11_SUBRESOURCE_DATA subres_init;
    D3D11_SUBRESOURCE_DATA* p_subres = nullptr;
    if (init_data != nullptr)
    {
        subres_init.pSysMem = init_data;
        subres_init.SysMemPitch = size_in_byte_;
        subres_init.SysMemSlicePitch = size_in_byte_;

        p_subres = &subres_init;
    }

    D3D11_BUFFER_DESC desc = {};
    GetD3DFlags(desc.Usage, desc.CPUAccessFlags, desc.BindFlags, desc.MiscFlags);
    desc.ByteWidth = size_in_byte_;
    desc.StructureByteStride = structure_byte_stride_;

    TIFHR(d3d_device_->CreateBuffer(&desc, p_subres, d3d_buffer_.put()));
}

void D3D11GraphicsBuffer::DeleteHWResource()
{
    d3d_buffer_.reset();
}

bool D3D11GraphicsBuffer::HWResourceReady() const 
{
    return d3d_buffer_.get() ? true : false;
}

void D3D11GraphicsBuffer::UpdateSubresource(uint32_t offset, uint32_t size, void const * data)
{
    D3D11_BOX* p = nullptr;
    D3D11_BOX box;
    if (!(bind_flags_ & D3D11_BIND_CONSTANT_BUFFER))
    {
        p = &box;
        box.left = offset;
        box.top = 0;
        box.front = 0;
        box.right = offset + size;
        box.bottom = 1;
        box.back = 1;
    }
    d3d_imm_ctx_->UpdateSubresource(d3d_buffer_.get(), 0, p, data, size, size);
}

void* D3D11GraphicsBuffer::Map(BufferAccess ba)
{
    COMMON_ASSERT(d3d_buffer_);

    D3D11_MAP type;
    switch (ba)
    {
    case BA_Read_Only:
        type = D3D11_MAP_READ;
        break;

    case BA_Write_Only:
        if ((EAH_CPU_Write == access_hint_) || ((EAH_CPU_Write | EAH_GPU_Read) == access_hint_))
        {
            type = D3D11_MAP_WRITE_DISCARD;
        }
        else
        {
            type = D3D11_MAP_WRITE;
        }
        break;

    case BA_Read_Write:
        type = D3D11_MAP_READ_WRITE;
        break;

    case BA_Write_No_Overwrite:
        type = D3D11_MAP_WRITE_NO_OVERWRITE;
        break;
    }

    D3D11_MAPPED_SUBRESOURCE mapped;
    TIFHR(d3d_imm_ctx_->Map(d3d_buffer_.get(), 0, type, 0, &mapped));
    return mapped.pData;
}

void D3D11GraphicsBuffer::Unmap()
{
    COMMON_ASSERT(d3d_buffer_);

    d3d_imm_ctx_->Unmap(d3d_buffer_.get(), 0);
}
}