#include "D3D11Texture.h"
#include <base/ZEngine.h>
#include <render/ElementFormat.h>
#include <render/TexCompression.h>
#include "D3D11Util.h"
#include "D3D11RenderEngine.h"

namespace RenderWorker
{

D3D11Texture3D::D3D11Texture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t numMipMaps, uint32_t array_size, 
        ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
    :D3D11Texture(TT_1D, sample_count, sample_quality, access_hint)
{
    COMMON_ASSERT(1 == array_size);

    if (0 == numMipMaps)
    {
        numMipMaps = 1;
        uint32_t w = width;
        uint32_t h = height;
        uint32_t d = depth;
        while ((w != 1) || (h != 1) || (d != 1))
        {
            ++ numMipMaps;

            w = std::max(1U, w / 2);
            h = std::max(1U, h / 2);
            d = std::max(1U, d / 2);
        }
    }
    mip_maps_num_ = numMipMaps;

    array_size_ = array_size;
    format_		= format;
    dxgi_fmt_ = D3D11Mapping::MappingFormat(format_);

    width_ = width;
    height_ = height;
    depth_ = depth;
}

uint32_t D3D11Texture3D::Width(uint32_t level) const noexcept 
{
    COMMON_ASSERT(level < mip_maps_num_);
    return std::max(1U, width_ >> level);
}

uint32_t D3D11Texture3D::Height(uint32_t level) const noexcept 
{
    COMMON_ASSERT(level < mip_maps_num_);
    return std::max(1U, height_ >> level);
}

uint32_t D3D11Texture3D::Depth(uint32_t level) const noexcept 
{
    COMMON_ASSERT(level < mip_maps_num_);
    return std::max(1U, depth_ >> level);
}

void D3D11Texture3D::CreateHWResource(std::span<ElementInitData const> init_data, float4 const * clear_value_hint)
{
    D3D11_TEXTURE3D_DESC desc;
    desc.Width = width_;
    desc.Height = height_;
    desc.Depth = depth_;
    desc.MipLevels = mip_maps_num_;
    desc.Format = dxgi_fmt_;
    this->GetD3DFlags(desc.Usage, desc.BindFlags, desc.CPUAccessFlags, desc.MiscFlags);

    std::vector<D3D11_SUBRESOURCE_DATA> subres_data;
    if (!init_data.empty())
    {
        COMMON_ASSERT(init_data.size() == mip_maps_num_);
        subres_data.resize(init_data.size());
        for (size_t i = 0; i < init_data.size(); ++i)
        {
            subres_data[i].pSysMem = init_data[i].data;
            subres_data[i].SysMemPitch = init_data[i].row_pitch;
            subres_data[i].SysMemSlicePitch = init_data[i].slice_pitch;
        }
    }

    ID3D11Texture3DPtr d3d_tex;
    TIFHR(d3d_device_->CreateTexture3D(&desc, subres_data.data(), d3d_tex.put()));
    d3d_tex.as(d3d_texture_);
}

void D3D11Texture3D::Map3D(uint32_t array_index, uint32_t level, TextureMapAccess tma,
    uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
    uint32_t /*width*/, uint32_t /*height*/, uint32_t /*depth*/,
    void*& data, uint32_t& row_pitch, uint32_t& slice_pitch)
{
    D3D11_MAPPED_SUBRESOURCE mapped;
    TIFHR(d3d_imm_ctx_->Map(d3d_texture_.get(), 
        D3D11CalcSubresource(level, array_index, mip_maps_num_), 
        D3D11Mapping::Mapping(tma, type_, access_hint_, mip_maps_num_), 0, &mapped));
    uint8_t* p = static_cast<uint8_t*>(mapped.pData);
    if (IsCompressedFormat(format_))
    {
        uint32_t const block_width = BlockWidth(format_);
        uint32_t const block_height = BlockHeight(format_);
        uint32_t const block_depth = BlockDepth(format_);
        uint32_t const block_bytes = BlockBytes(format_);
        data = p + (z_offset / block_depth) * mapped.DepthPitch
            + (y_offset / block_height) * mapped.RowPitch + (x_offset / block_width) * block_bytes;
    }
    else
    {
        data = p + z_offset * mapped.DepthPitch + y_offset * mapped.RowPitch + x_offset * NumFormatBytes(format_);
    }
    row_pitch = mapped.RowPitch;
    slice_pitch = mapped.DepthPitch;
}

void D3D11Texture3D::Unmap3D(uint32_t array_index, uint32_t level)
{
    d3d_imm_ctx_->Unmap(d3d_texture_.get(), D3D11CalcSubresource(level, array_index, mip_maps_num_));
}

D3D11_SHADER_RESOURCE_VIEW_DESC D3D11Texture3D::FillSRVDesc(ElementFormat pf, 
    uint32_t first_array_index, uint32_t array_size, uint32_t first_level, uint32_t num_levels) const 
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

    desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
    desc.Texture3D.MostDetailedMip = first_level;
    desc.Texture3D.MipLevels = num_levels;

    return desc;
}

D3D11_RENDER_TARGET_VIEW_DESC D3D11Texture3D::FillRTVDesc(ElementFormat pf, 
    uint32_t array_index, uint32_t first_slice, uint32_t num_slices, uint32_t level) const 
{
    COMMON_ASSERT(0 == array_index);

    D3D11_RENDER_TARGET_VIEW_DESC desc;
    desc.Format = D3D11Mapping::MappingFormat(pf);
    desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
    desc.Texture3D.MipSlice = level;
    desc.Texture3D.FirstWSlice = first_slice;
    desc.Texture3D.WSize = num_slices;

    return desc;
}

}