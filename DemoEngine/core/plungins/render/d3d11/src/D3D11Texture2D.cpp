#include "D3D11Texture.h"
#include <base/ZEngine.h>
#include <render/ElementFormat.h>
#include "D3D11Util.h"
#include "D3D11RenderEngine.h"

namespace RenderWorker
{

D3D11Texture2D::D3D11Texture2D(uint32_t width, uint32_t height, uint32_t num_mip_maps, uint32_t array_size, ElementFormat format,
    uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
    :D3D11Texture(TT_2D, sample_count, sample_quality, access_hint)
{
    if (0 == num_mip_maps)
    {
        num_mip_maps = 1;
        uint32_t w = width;
        uint32_t h = height;
        while ((w != 1) || (h != 1))
        {
        ++ num_mip_maps;

        w = std::max(1U, w / 2);
        h = std::max(1U, h / 2);
        }
    }
    mip_maps_num_ = num_mip_maps;

    array_size_ = array_size;
    format_		= format;
    dxgi_fmt_ = D3D11Mapping::MappingFormat(format_);
    width_ = width;
    height_ = height;
}

D3D11Texture2D::D3D11Texture2D(const ID3D11Texture2DPtr& d3d_tex)
    : D3D11Texture(TT_2D, 1, 0, 0)
{
    D3D11_TEXTURE2D_DESC desc;
    d3d_tex->GetDesc(&desc);

    mip_maps_num_ = desc.MipLevels;
    array_size_ = desc.ArraySize;
    format_ = D3D11Mapping::MappingFormat(desc.Format);
    sample_count_ = desc.SampleDesc.Count;
    sample_quality_ = desc.SampleDesc.Quality;
    width_ = desc.Width;
    height_ = desc.Height;

    access_hint_ = 0;
    switch (desc.Usage)
    {
    case D3D11_USAGE_DEFAULT:
        access_hint_ |= EAH_GPU_Read | EAH_GPU_Write;
        break;

    case D3D11_USAGE_IMMUTABLE:
        access_hint_ |= EAH_Immutable;
        break;

    case D3D11_USAGE_DYNAMIC:
        access_hint_ |= EAH_GPU_Read;
        if (desc.CPUAccessFlags & D3D11_CPU_ACCESS_WRITE)
        {
            access_hint_ |= EAH_CPU_Write;
        }
        break;

    case D3D11_USAGE_STAGING:
        if (desc.CPUAccessFlags & D3D11_CPU_ACCESS_READ)
        {
            access_hint_ |= EAH_CPU_Read;
        }
        if (desc.CPUAccessFlags & D3D11_CPU_ACCESS_WRITE)
        {
            access_hint_ |= EAH_CPU_Write;
        }
        break;

    default:
        ZENGINE_UNREACHABLE("Invalid usage");
    }
    if (desc.BindFlags & D3D11_BIND_UNORDERED_ACCESS)
    {
        access_hint_ |= EAH_GPU_Unordered;
    }
    if (desc.MiscFlags & D3D11_RESOURCE_MISC_GENERATE_MIPS)
    {
        access_hint_ |= EAH_Generate_Mips;
    }

    d3d_texture_ = d3d_tex;
}

void D3D11Texture2D::CreateHWResource(std::span<ElementInitData const> init_data, float4 const * clear_value_hint)
{
    D3D11_TEXTURE2D_DESC desc;
    desc.Width = width_;
    desc.Height = height_;
    desc.MipLevels = mip_maps_num_;
    desc.ArraySize = array_size_;
    switch (format_)
    {
    case EF_D16:
        desc.Format = DXGI_FORMAT_R16_TYPELESS;
        break;

    case EF_D24S8:
        desc.Format = DXGI_FORMAT_R24G8_TYPELESS;
        break;

    case EF_D32F:
        desc.Format = DXGI_FORMAT_R32_TYPELESS;
        break;

    default:
        desc.Format = dxgi_fmt_;
        break;
    }
    desc.SampleDesc.Count = sample_count_;
    desc.SampleDesc.Quality = sample_quality_;
    this->GetD3DFlags(desc.Usage, desc.BindFlags, desc.CPUAccessFlags, desc.MiscFlags);

    std::vector<D3D11_SUBRESOURCE_DATA> subres_data;
    if (!init_data.empty())
    {
        COMMON_ASSERT(init_data.size() == array_size_ * mip_maps_num_);
        subres_data.resize(init_data.size());
        for (size_t i = 0; i < init_data.size(); ++i)
        {
            subres_data[i].pSysMem = init_data[i].data;
            subres_data[i].SysMemPitch = init_data[i].row_pitch;
            subres_data[i].SysMemSlicePitch = init_data[i].slice_pitch;
        }
    }

    ID3D11Texture2DPtr d3d_tex;
    TIFHR(d3d_device_->CreateTexture2D(&desc, subres_data.data(), d3d_tex.put()));
    d3d_tex.as(d3d_texture_);
}

uint32_t D3D11Texture2D::Width(uint32_t level) const noexcept 
{
    COMMON_ASSERT(level < mip_maps_num_);
    return std::max(1U, width_ >> level);
}

uint32_t D3D11Texture2D::Height(uint32_t level) const noexcept 
{
    COMMON_ASSERT(level < mip_maps_num_);
    return std::max(1U, height_ >> level);
}

D3D11_SHADER_RESOURCE_VIEW_DESC D3D11Texture2D::FillSRVDesc(ElementFormat pf, 
    uint32_t first_array_index, uint32_t array_size, uint32_t first_level, uint32_t num_levels) const
{
    D3D11_SHADER_RESOURCE_VIEW_DESC desc;
    switch (pf)
    {
    case EF_D16:
        desc.Format = DXGI_FORMAT_R16_UNORM;
        break;

    case EF_D24S8:
        desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
        break;

    case EF_D32F:
        desc.Format = DXGI_FORMAT_R32_FLOAT;
        break;

    default:
        desc.Format = D3D11Mapping::MappingFormat(pf);
        break;
    }

    if (array_size_ > 1)
    {
        if (sample_count_ > 1)
        {
            desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY;
            desc.Texture2DMSArray.FirstArraySlice = first_array_index;
            desc.Texture2DMSArray.ArraySize = array_size;
        }
        else
        {
            desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
            desc.Texture2DArray.MostDetailedMip = first_level;
            desc.Texture2DArray.MipLevels = num_levels;
            desc.Texture2DArray.FirstArraySlice = first_array_index;
            desc.Texture2DArray.ArraySize = array_size;
        }
    }
    else
    {
        if (sample_count_ > 1)
        {
            desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
        }
        else
        {
            desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MostDetailedMip = first_level;
            desc.Texture2D.MipLevels = num_levels;
        }
    }

    return desc;
}

D3D11_RENDER_TARGET_VIEW_DESC D3D11Texture2D::FillRTVDesc(ElementFormat pf, 
    uint32_t first_array_index, uint32_t array_size, uint32_t level) const 
{
    D3D11_RENDER_TARGET_VIEW_DESC desc;
    desc.Format = D3D11Mapping::MappingFormat(pf);
    if (array_size > 1)
    {
        if (sample_count_ > 1)
        {
            desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
            desc.Texture2DMSArray.FirstArraySlice = first_array_index;
            desc.Texture2DMSArray.ArraySize = array_size;
        }
        else
        {
            desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
            desc.Texture2DArray.MipSlice = level;
            desc.Texture2DArray.FirstArraySlice = first_array_index;
            desc.Texture2DArray.ArraySize = array_size;
        }
    }
    else
    {
        if (sample_count_ > 1)
        {
            desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
        }
        else
        {
            desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MipSlice = level;
        }
    }

    return desc;
}

D3D11_DEPTH_STENCIL_VIEW_DESC D3D11Texture2D::FillDSVDesc(ElementFormat pf, 
    uint32_t first_array_index, uint32_t array_size, uint32_t level) const 
{
    D3D11_DEPTH_STENCIL_VIEW_DESC desc;
    desc.Format = D3D11Mapping::MappingFormat(pf);
    desc.Flags = 0;
    if (array_size_ > 1)
    {
        if (sample_count_ > 1)
        {
            desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;
            desc.Texture2DMSArray.FirstArraySlice = first_array_index;
            desc.Texture2DMSArray.ArraySize = array_size;
        }
        else
        {
            desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
            desc.Texture2DArray.MipSlice = level;
            desc.Texture2DArray.FirstArraySlice = first_array_index;
            desc.Texture2DArray.ArraySize = array_size;
        }
    }
    else
    {
        if (sample_count_ > 1)
        {
            desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
        }
        else
        {
            desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MipSlice = level;
        }
    }

    return desc;
}
}