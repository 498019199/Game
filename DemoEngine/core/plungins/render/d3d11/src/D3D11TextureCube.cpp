#include "D3D11Texture.h"
#include <common/Util.h>
#include <base/Context.h>
#include <render/ElementFormat.h>
#include "D3D11Util.h"
#include "D3D11RenderEngine.h"

namespace RenderWorker
{
D3D11TextureCube::D3D11TextureCube(uint32_t size, uint32_t numMipMaps, uint32_t array_size, ElementFormat format,
        uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
    :D3D11Texture(TT_1D, sample_count, sample_quality, access_hint)
{
    if (0 == numMipMaps)
    {
        numMipMaps = 1;
        uint32_t w = size;
        while (w != 1)
        {
            ++ numMipMaps;

            w = std::max(1U, w / 2);
        }
    }
    mip_maps_num_ = numMipMaps;

    array_size_ = array_size;
    format_		= format;
    dxgi_fmt_ = D3D11Mapping::MappingFormat(format_);

    width_ = size;
}

uint32_t D3D11TextureCube::Width(uint32_t level) const noexcept 
{
	COMMON_ASSERT(level < mip_maps_num_);
	return std::max(1U, width_ >> level);
}

uint32_t D3D11TextureCube::Height(uint32_t level) const noexcept 
{
	return this->Width(level);
}

void D3D11TextureCube::CreateHWResource(std::span<ElementInitData const> init_data, float4 const * clear_value_hint)
{
    D3D11_TEXTURE2D_DESC desc;
    desc.Width = width_;
    desc.Height = width_;
    desc.MipLevels = mip_maps_num_;
    desc.ArraySize = 6 * array_size_;
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
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    this->GetD3DFlags(desc.Usage, desc.BindFlags, desc.CPUAccessFlags, desc.MiscFlags);
    desc.MiscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;

    std::vector<D3D11_SUBRESOURCE_DATA> subres_data;
    if (!init_data.empty())
    {
        COMMON_ASSERT(init_data.size() == 6 * array_size_ * mip_maps_num_);
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

D3D11_SHADER_RESOURCE_VIEW_DESC D3D11TextureCube::FillSRVDesc(ElementFormat pf,  [[maybe_unused]] uint32_t first_array_index, 
    [[maybe_unused]] uint32_t array_size, uint32_t first_level, uint32_t num_levels) const 
{
    COMMON_ASSERT(0 == first_array_index);
    COMMON_ASSERT(1 == array_size);

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

    desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    desc.TextureCube.MostDetailedMip = first_level;
    desc.TextureCube.MipLevels = num_levels;

    return desc;
}

D3D11_SHADER_RESOURCE_VIEW_DESC D3D11TextureCube::FillSRVDesc(ElementFormat pf, 
    uint32_t array_index, CubeFaces face, uint32_t first_level, uint32_t num_levels) const 
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

    desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    desc.Texture2DArray.MostDetailedMip = first_level;
    desc.Texture2DArray.MipLevels = num_levels;
    desc.Texture2DArray.FirstArraySlice = array_index * 6 + face - CF_Positive_X;
    desc.Texture2DArray.ArraySize = 1;

    return desc;
}

D3D11_RENDER_TARGET_VIEW_DESC D3D11TextureCube::FillRTVDesc(ElementFormat pf, 
    uint32_t first_array_index, uint32_t array_size, uint32_t level) const 
{
    D3D11_RENDER_TARGET_VIEW_DESC desc;
    desc.Format = D3D11Mapping::MappingFormat(pf);
    if (this->SampleCount() > 1)
    {
        desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
    }
    else
    {
        desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
    }
    desc.Texture2DArray.MipSlice = level;
    desc.Texture2DArray.FirstArraySlice = first_array_index * 6;
    desc.Texture2DArray.ArraySize = array_size * 6;

    return desc;
}

D3D11_RENDER_TARGET_VIEW_DESC D3D11TextureCube::FillRTVDesc(ElementFormat pf, 
    uint32_t array_index, CubeFaces face, uint32_t level) const 
{
    D3D11_RENDER_TARGET_VIEW_DESC desc;
    desc.Format = D3D11Mapping::MappingFormat(pf);
    if (this->SampleCount() > 1)
    {
        desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
    }
    else
    {
        desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
    }
    desc.Texture2DArray.MipSlice = level;
    desc.Texture2DArray.FirstArraySlice = array_index * 6 + face - CF_Positive_X;
    desc.Texture2DArray.ArraySize = 1;

    return desc;
}

}