#include "D3D11Texture.h"
#include <base/ZEngine.h>
#include <render/ElementFormat.h>
#include <render/TexCompression.h>
#include "D3D11Util.h"
#include "D3D11RenderEngine.h"

namespace RenderWorker
{
D3D11TextureCube::D3D11TextureCube(uint32_t size, uint32_t MipMapsNum, uint32_t array_size, ElementFormat format,
        uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
    :D3D11Texture(TT_1D, sample_count, sample_quality, access_hint)
{
    if (0 == MipMapsNum)
    {
        MipMapsNum = 1;
        uint32_t w = size;
        while (w != 1)
        {
            ++ MipMapsNum;

            w = std::max(1U, w / 2);
        }
    }
    mip_maps_num_ = MipMapsNum;

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

void D3D11TextureCube::MapCube(uint32_t array_index, CubeFaces face, uint32_t level, TextureMapAccess tma,
    uint32_t x_offset, uint32_t y_offset, uint32_t /*width*/, uint32_t /*height*/,
    void*& data, uint32_t& row_pitch)
{
    D3D11_MAPPED_SUBRESOURCE mapped;
    TIFHR(d3d_imm_ctx_->Map(d3d_texture_.get(), D3D11CalcSubresource(level, array_index * 6 + face - CF_Positive_X, mip_maps_num_),
        D3D11Mapping::Mapping(tma, type_, access_hint_, mip_maps_num_), 0, &mapped));
    uint8_t* p = static_cast<uint8_t*>(mapped.pData);
    if (IsCompressedFormat(format_))
    {
        uint32_t const block_width = BlockWidth(format_);
        uint32_t const block_height = BlockHeight(format_);
        uint32_t const block_bytes = BlockBytes(format_);
        data = p + (y_offset / block_height) * mapped.RowPitch + (x_offset / block_width) * block_bytes;
    }
    else
    {
        data = p + y_offset * mapped.RowPitch + x_offset * NumFormatBytes(format_);
    }
    row_pitch = mapped.RowPitch;
}

void D3D11TextureCube::UnmapCube(uint32_t array_index, CubeFaces face, uint32_t level)
{
    d3d_imm_ctx_->Unmap(d3d_texture_.get(), D3D11CalcSubresource(level, array_index * 6 + face - CF_Positive_X, mip_maps_num_));
}

void D3D11TextureCube::CopyToTexture(Texture& target, TextureFilter filter)
{
    COMMON_ASSERT(type_ == target.Type());

    if ((this->Width(0) == target.Width(0)) && (this->Format() == target.Format())
        && (this->ArraySize() == target.ArraySize()) && (this->MipMapsNum() == target.MipMapsNum()))
    {
        auto& other = checked_cast<D3D11TextureCube&>(target);
        d3d_imm_ctx_->CopyResource(other.d3d_texture_.get(), d3d_texture_.get());
    }
    else
    {
        uint32_t const array_size = std::min(this->ArraySize(), target.ArraySize());
        uint32_t const num_mips = std::min(this->MipMapsNum(), target.MipMapsNum());
        for (uint32_t index = 0; index < array_size; ++ index)
        {
            for (int f = 0; f < 6; ++ f)
            {
                CubeFaces const face = static_cast<CubeFaces>(f);
                for (uint32_t level = 0; level < num_mips; ++ level)
                {
                    this->ResizeTextureCube(target, index, face, level, 0, 0, target.Width(level), target.Height(level), index, face,
                        level, 0, 0, this->Width(level), this->Height(level), filter);
                }
            }
        }
    }
}

void D3D11TextureCube::CopyToSubTexture2D(Texture& target, uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset,
    uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height, uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset,
    uint32_t src_y_offset, uint32_t src_width, uint32_t src_height, TextureFilter filter)
{
    COMMON_ASSERT((TT_2D == target.Type()) || (TT_Cube == target.Type()));

    if ((src_width == dst_width) && (src_height == dst_height) && (this->Format() == target.Format()))
    {
        auto& other = checked_cast<D3D11Texture&>(target);

        D3D11_BOX* src_box_ptr;
        D3D11_BOX src_box;
        if ((sample_count_ != 1) || IsDepthFormat(format_))
        {
            COMMON_ASSERT(other.SampleCount() == sample_count_);
            COMMON_ASSERT(dst_x_offset == 0);
            COMMON_ASSERT(dst_y_offset == 0);

            src_box_ptr = nullptr;
        }
        else
        {
            src_box.left = src_x_offset;
            src_box.top = src_y_offset;
            src_box.front = 0;
            src_box.right = src_x_offset + src_width;
            src_box.bottom = src_y_offset + src_height;
            src_box.back = 1;

            src_box_ptr = &src_box;
        }

        d3d_imm_ctx_->CopySubresourceRegion(other.D3DResource(), D3D11CalcSubresource(dst_level, dst_array_index, target.MipMapsNum()),
            dst_x_offset, dst_y_offset, 0, this->D3DResource(), D3D11CalcSubresource(src_level, src_array_index, this->MipMapsNum()),
            src_box_ptr);
    }
    else
    {
        this->ResizeTexture2D(target, dst_array_index, dst_level, dst_x_offset, dst_y_offset, dst_width, dst_height,
            src_array_index, src_level, src_x_offset, src_y_offset, src_width, src_height, filter);
    }
}

void D3D11TextureCube::CopyToSubTextureCube(Texture& target, uint32_t dst_array_index, CubeFaces dst_face, uint32_t dst_level,
    uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height, uint32_t src_array_index, CubeFaces src_face,
    uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_width, uint32_t src_height, TextureFilter filter)
{
    COMMON_ASSERT(type_ == target.Type());

    if ((src_width == dst_width) && (src_height == dst_height) && (this->Format() == target.Format()))
    {
        auto& other = checked_cast<D3D11Texture&>(target);

        D3D11_BOX* src_box_ptr;
        D3D11_BOX src_box;
        if ((sample_count_ != 1) || IsDepthFormat(format_))
        {
            COMMON_ASSERT(other.SampleCount() == sample_count_);
            COMMON_ASSERT(dst_x_offset == 0);
            COMMON_ASSERT(dst_y_offset == 0);

            src_box_ptr = nullptr;
        }
        else
        {
            src_box.left = src_x_offset;
            src_box.top = src_y_offset;
            src_box.front = 0;
            src_box.right = src_x_offset + src_width;
            src_box.bottom = src_y_offset + src_height;
            src_box.back = 1;

            src_box_ptr = &src_box;
        }

        d3d_imm_ctx_->CopySubresourceRegion(other.D3DResource(),
            D3D11CalcSubresource(dst_level, dst_array_index * 6 + dst_face - CF_Positive_X, target.MipMapsNum()),
            dst_x_offset, dst_y_offset, 0, this->D3DResource(),
            D3D11CalcSubresource(src_level, src_array_index * 6 + src_face - CF_Positive_X, this->MipMapsNum()),
            src_box_ptr);
    }
    else
    {
        this->ResizeTextureCube(target, dst_array_index, dst_face, dst_level, dst_x_offset, dst_y_offset, dst_width, dst_height,
            src_array_index, src_face, src_level, src_x_offset, src_y_offset, src_width, src_height, filter);
    }
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