#include "D3D11Texture.h"

#include <base/Context.h>
#include <common/Hash.h>
#include <render/ElementFormat.h>

#include "D3D11RenderEngine.h"

namespace RenderWorker
{
using namespace CommonWorker;

D3D11Texture::D3D11Texture(TextureType type, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
    :Texture(type, sample_count, sample_quality, access_hint)
{
    if (access_hint & EAH_GPU_Write)
    {
        COMMON_ASSERT(!(access_hint & EAH_CPU_Read));
        COMMON_ASSERT(!(access_hint & EAH_CPU_Write));
    }

    const auto& d3d11_re = checked_cast<const D3D11RenderEngine&>(Context::Instance().RenderEngineInstance());
    d3d_device_ = d3d11_re.D3DDevice1();
    d3d_imm_ctx_ = d3d11_re.D3DDeviceImmContext1();
}

#ifndef KLAYGE_SHIP
void D3D11Texture::DebugName(std::wstring_view name)
{
    d3d_texture_->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<uint32_t>(name.size() * sizeof(wchar_t)), name.data());
}
#endif 

ID3D11Resource* D3D11Texture::D3DResource() const noexcept
{
    return d3d_texture_.get();
}

void D3D11Texture::DeleteHWResource()
{
    d3d_texture_.reset();
}

bool D3D11Texture::HWResourceReady() const 
{
    return d3d_texture_.get() ? true : false;
}

uint32_t D3D11Texture::Width(uint32_t level) const noexcept 
{
    COMMON_ASSERT(level < mip_maps_num_);
    return 1;
}

uint32_t D3D11Texture::Height(uint32_t level) const noexcept 
{
    COMMON_ASSERT(level < mip_maps_num_);
    return 1;
}

uint32_t D3D11Texture::Depth(uint32_t level) const noexcept 
{
    COMMON_ASSERT(level < mip_maps_num_);
    return 1;
}

const ID3D11RenderTargetViewPtr& D3D11Texture::RetrieveD3DRenderTargetView(ElementFormat pf, uint32_t first_array_index, uint32_t array_size,
        uint32_t level)
{
    COMMON_ASSERT(this->AccessHint() & EAH_GPU_Write);
    COMMON_ASSERT(first_array_index < this->ArraySize());
    COMMON_ASSERT(first_array_index + array_size <= this->ArraySize());

    size_t hash_val = HashValue(pf);
    HashCombine(hash_val, first_array_index);
    HashCombine(hash_val, array_size);
    HashCombine(hash_val, level);
    HashCombine(hash_val, 0);
    HashCombine(hash_val, 0);

    auto iter = d3d_rt_views_.find(hash_val);
    if (iter != d3d_rt_views_.end())
    {
        return iter->second;
    }
    else
    {
        auto desc = this->FillRTVDesc(pf, first_array_index, array_size, level);
        ID3D11RenderTargetViewPtr d3d_rt_view;
        d3d_device_->CreateRenderTargetView(this->D3DResource(), &desc, d3d_rt_view.put());
        return d3d_rt_views_.emplace(hash_val, std::move(d3d_rt_view)).first->second;
    }
}

const ID3D11RenderTargetViewPtr& D3D11Texture::RetrieveD3DRenderTargetView(ElementFormat pf, uint32_t array_index, uint32_t first_slice,
        uint32_t num_slices, uint32_t level)
{
    COMMON_ASSERT(this->AccessHint() & EAH_GPU_Write);
    COMMON_ASSERT(0 == array_index);

    size_t hash_val = HashValue(pf);
    HashCombine(hash_val, array_index);
    HashCombine(hash_val, 1);
    HashCombine(hash_val, level);
    HashCombine(hash_val, first_slice);
    HashCombine(hash_val, num_slices);

    auto iter = d3d_rt_views_.find(hash_val);
    if (iter != d3d_rt_views_.end())
    {
        return iter->second;
    }
    else
    {
        auto desc = this->FillRTVDesc(pf, array_index, first_slice, num_slices, level);
        ID3D11RenderTargetViewPtr d3d_rt_view;
        d3d_device_->CreateRenderTargetView(this->D3DResource(), &desc, d3d_rt_view.put());
        return d3d_rt_views_.emplace(hash_val, std::move(d3d_rt_view)).first->second;
    }
}

const ID3D11RenderTargetViewPtr& D3D11Texture::RetrieveD3DRenderTargetView(ElementFormat pf, uint32_t array_index, CubeFaces face,
        uint32_t level)
{
    COMMON_ASSERT(this->AccessHint() & EAH_GPU_Write);

    size_t hash_val = HashValue(pf);
    HashCombine(hash_val, array_index * 6 + face);
    HashCombine(hash_val, 1);
    HashCombine(hash_val, level);
    HashCombine(hash_val, 0);
    HashCombine(hash_val, 0);

    auto iter = d3d_rt_views_.find(hash_val);
    if (iter != d3d_rt_views_.end())
    {
        return iter->second;
    }
    else
    {
        auto desc = this->FillRTVDesc(pf, array_index, face, level);
        ID3D11RenderTargetViewPtr d3d_rt_view;
        d3d_device_->CreateRenderTargetView(this->D3DResource(), &desc, d3d_rt_view.put());
        return d3d_rt_views_.emplace(hash_val, std::move(d3d_rt_view)).first->second;
    }
}

const ID3D11DepthStencilViewPtr& D3D11Texture::RetrieveD3DDepthStencilView(ElementFormat pf, uint32_t first_array_index, uint32_t array_size,
        uint32_t level)
{
    COMMON_ASSERT(this->AccessHint() & EAH_GPU_Write);
    COMMON_ASSERT(first_array_index < this->ArraySize());
    COMMON_ASSERT(first_array_index + array_size <= this->ArraySize());

    size_t hash_val = HashValue(pf);
    HashCombine(hash_val, first_array_index);
    HashCombine(hash_val, array_size);
    HashCombine(hash_val, level);
    HashCombine(hash_val, 0);
    HashCombine(hash_val, 0);

    auto iter = d3d_ds_views_.find(hash_val);
    if (iter != d3d_ds_views_.end())
    {
        return iter->second;
    }
    else
    {
        auto desc = this->FillDSVDesc(pf, first_array_index, array_size, level);
        ID3D11DepthStencilViewPtr d3d_ds_view;
        d3d_device_->CreateDepthStencilView(this->D3DResource(), &desc, d3d_ds_view.put());
        return d3d_ds_views_.emplace(hash_val, std::move(d3d_ds_view)).first->second;
    }
}

const ID3D11DepthStencilViewPtr& D3D11Texture::RetrieveD3DDepthStencilView(ElementFormat pf, uint32_t array_index, uint32_t first_slice,
        uint32_t num_slices, uint32_t level)
{
    COMMON_ASSERT(this->AccessHint() & EAH_GPU_Write);
    COMMON_ASSERT(0 == array_index);

    size_t hash_val = HashValue(pf);
    HashCombine(hash_val, array_index);
    HashCombine(hash_val, 1);
    HashCombine(hash_val, level);
    HashCombine(hash_val, first_slice);
    HashCombine(hash_val, num_slices);

    auto iter = d3d_ds_views_.find(hash_val);
    if (iter != d3d_ds_views_.end())
    {
        return iter->second;
    }
    else
    {
        auto desc = this->FillDSVDesc(pf, array_index, first_slice, num_slices, level);
        ID3D11DepthStencilViewPtr d3d_ds_view;
        d3d_device_->CreateDepthStencilView(this->D3DResource(), &desc, d3d_ds_view.put());
        return d3d_ds_views_.emplace(hash_val, std::move(d3d_ds_view)).first->second;
    }
}

const ID3D11DepthStencilViewPtr& D3D11Texture::RetrieveD3DDepthStencilView(ElementFormat pf, uint32_t array_index, CubeFaces face,
        uint32_t level)
{
    COMMON_ASSERT(this->AccessHint() & EAH_GPU_Write);

    size_t hash_val = HashValue(pf);
    HashCombine(hash_val, array_index * 6 + face);
    HashCombine(hash_val, 1);
    HashCombine(hash_val, level);
    HashCombine(hash_val, 0);
    HashCombine(hash_val, 0);

    auto iter = d3d_ds_views_.find(hash_val);
    if (iter != d3d_ds_views_.end())
    {
        return iter->second;
    }
    else
    {
        auto desc = this->FillDSVDesc(pf, array_index, face, level);
        ID3D11DepthStencilViewPtr d3d_ds_view;
        d3d_device_->CreateDepthStencilView(this->D3DResource(), &desc, d3d_ds_view.put());
        return d3d_ds_views_.emplace(hash_val, std::move(d3d_ds_view)).first->second;
    }
}

ID3D11ShaderResourceViewPtr const & D3D11Texture::RetrieveD3DShaderResourceView(ElementFormat pf, uint32_t first_array_index,
    uint32_t array_size, uint32_t first_level, uint32_t num_levels)
{
    COMMON_ASSERT(this->AccessHint() & EAH_GPU_Read);

    size_t hash_val = HashValue(pf);
    HashCombine(hash_val, first_array_index);
    HashCombine(hash_val, array_size);
    HashCombine(hash_val, first_level);
    HashCombine(hash_val, num_levels);

    auto iter = d3d_sr_views_.find(hash_val);
    if (iter != d3d_sr_views_.end())
    {
        return iter->second;
    }
    else
    {
        auto desc = this->FillSRVDesc(pf, first_array_index, array_size, first_level, num_levels);
        ID3D11ShaderResourceViewPtr d3d_sr_view;
        d3d_device_->CreateShaderResourceView(this->D3DResource(), &desc, d3d_sr_view.put());
        return d3d_sr_views_.emplace(hash_val, std::move(d3d_sr_view)).first->second;
    }
}

void D3D11Texture::GetD3DFlags(D3D11_USAGE& usage, UINT& bind_flags, UINT& cpu_access_flags, UINT& misc_flags)
{
    if (access_hint_ & EAH_Immutable)
    {
        usage = D3D11_USAGE_IMMUTABLE;
    }
    else
    {
        if ((EAH_CPU_Write | EAH_GPU_Read) == access_hint_)
        {
            usage = D3D11_USAGE_DYNAMIC;
        }
        else
        {
            if (EAH_CPU_Write == access_hint_)
            {
                if ((mip_maps_num_ != 1) || (TT_Cube == type_))
                {
                    usage = D3D11_USAGE_STAGING;
                }
                else
                {
                    usage = D3D11_USAGE_DYNAMIC;
                }
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
    }

    bind_flags = 0;
    if ((access_hint_ & EAH_GPU_Read) || (D3D11_USAGE_DYNAMIC == usage))
    {
        bind_flags |= D3D11_BIND_SHADER_RESOURCE;
    }
    if (access_hint_ & EAH_GPU_Write)
    {
        if (IsDepthFormat(format_))
        {
            bind_flags |= D3D11_BIND_DEPTH_STENCIL;
        }
        else
        {
            bind_flags |= D3D11_BIND_RENDER_TARGET;
        }
    }
    if (access_hint_ & EAH_GPU_Unordered)
    {
        bind_flags |= D3D11_BIND_UNORDERED_ACCESS;
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

    misc_flags = 0;
    if (access_hint_ & EAH_Generate_Mips)
    {
        bind_flags |= D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        misc_flags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
    }
}

D3D11_SHADER_RESOURCE_VIEW_DESC D3D11Texture::FillSRVDesc(ElementFormat pf, 
    uint32_t first_array_index, uint32_t array_size, uint32_t first_level, uint32_t num_levels) const
{
    ZENGINE_UNREACHABLE("Can't be called");
}

D3D11_SHADER_RESOURCE_VIEW_DESC D3D11Texture::FillSRVDesc(ElementFormat pf, 
    uint32_t array_index, CubeFaces face, uint32_t first_level, uint32_t num_levels) const
{
    ZENGINE_UNREACHABLE("Can't be called");
}

D3D11_RENDER_TARGET_VIEW_DESC D3D11Texture::FillRTVDesc(ElementFormat pf, 
    uint32_t first_array_index, uint32_t array_size, uint32_t level) const
{
    ZENGINE_UNREACHABLE("Can't be called");
}

D3D11_RENDER_TARGET_VIEW_DESC D3D11Texture::FillRTVDesc(ElementFormat pf, 
    uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t level) const
{
    ZENGINE_UNREACHABLE("Can't be called");
}

D3D11_RENDER_TARGET_VIEW_DESC D3D11Texture::FillRTVDesc(ElementFormat pf, 
    uint32_t array_index, CubeFaces face, uint32_t level) const
{
    ZENGINE_UNREACHABLE("Can't be called");
}

D3D11_DEPTH_STENCIL_VIEW_DESC D3D11Texture::FillDSVDesc(ElementFormat pf, 
    uint32_t first_array_index, uint32_t array_size, uint32_t level) const
{
    ZENGINE_UNREACHABLE("Can't be called");
}

D3D11_DEPTH_STENCIL_VIEW_DESC D3D11Texture::FillDSVDesc(ElementFormat pf, 
    uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t level) const
{
    ZENGINE_UNREACHABLE("Can't be called");
}

D3D11_DEPTH_STENCIL_VIEW_DESC D3D11Texture::FillDSVDesc(ElementFormat pf, 
    uint32_t array_index, CubeFaces face, uint32_t level) const
{
    ZENGINE_UNREACHABLE("Can't be called");
}













}
