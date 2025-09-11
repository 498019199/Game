#include "D3D11Texture.h"
#include <base/ZEngine.h>
#include <render/ElementFormat.h>
#include "D3D11Util.h"
#include "D3D11RenderEngine.h"

namespace RenderWorker
{
D3D11Texture1D::D3D11Texture1D(uint32_t width, uint32_t numMipMaps, uint32_t array_size, ElementFormat format, 
        uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
    :D3D11Texture(TT_1D, sample_count, sample_quality, access_hint)
{
    if (0 == numMipMaps)
    {
        numMipMaps = 1;
        uint32_t w = width;
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
    width_ = width;
}

uint32_t D3D11Texture1D::Width(uint32_t level) const noexcept 
{
    COMMON_ASSERT(level < mip_maps_num_);
    return std::max(1U, width_ >> level);
}

void D3D11Texture1D::CreateHWResource(std::span<ElementInitData const> init_data, float4 const * clear_value_hint)
{
    D3D11_TEXTURE1D_DESC desc;
    desc.Width = width_;
    desc.MipLevels = mip_maps_num_;
    desc.ArraySize = array_size_;
    desc.Format = dxgi_fmt_;
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

    ID3D11Texture1DPtr texture;
    TIFHR(d3d_device_->CreateTexture1D(&desc, subres_data.data(), texture.put()));
    texture.as(d3d_texture_);
}

D3D11_SHADER_RESOURCE_VIEW_DESC D3D11Texture1D::FillSRVDesc(ElementFormat pf, 
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
        desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1DARRAY;
        desc.Texture1DArray.MostDetailedMip = first_level;
        desc.Texture1DArray.MipLevels = num_levels;
        desc.Texture1DArray.FirstArraySlice = first_array_index;
        desc.Texture1DArray.ArraySize = array_size;
    }
    else
    {
        desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
        desc.Texture1D.MostDetailedMip = first_level;
        desc.Texture1D.MipLevels = num_levels;
    }

    return desc;
}

D3D11_RENDER_TARGET_VIEW_DESC D3D11Texture1D::FillRTVDesc(ElementFormat pf, 
    uint32_t first_array_index, uint32_t array_size, uint32_t level) const 
{
    // 定义 RTV 描述
    D3D11_RENDER_TARGET_VIEW_DESC desc;
    desc.Format = D3D11Mapping::MappingFormat(pf);
    if (this->ArraySize() > 1)
    {
        desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1DARRAY; 
        desc.Texture1DArray.MipSlice = level;
        desc.Texture1DArray.FirstArraySlice = first_array_index;
        desc.Texture1DArray.ArraySize = array_size;
    }
    else
    {
        // 1D 纹理视图
        desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1D;
        // 渲染到第 0 层 Mip（最清晰层级）
        desc.Texture1D.MipSlice = level;
    }

    return desc;
}

}