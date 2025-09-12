
#include <dev_helps/TexMetadata.h>

namespace RenderWorker
{
TexMetadata::TexMetadata()
{
    
}

TexMetadata::TexMetadata(std::string_view name)
{
    
}

TexMetadata::TexMetadata(std::string_view name, bool assign_default_values)
{
    
}

void TexMetadata::Load(std::string_view name)
{
    Load(name, true);
}

void TexMetadata::Load(std::string_view name, bool assign_default_values)
{
    TexMetadata new_metadata;
}

void TexMetadata::Save(std::string const & name) const
{
    
}

void TexMetadata::DeviceDependentAdjustment(const RenderDeviceCaps& caps)
{
    // if (prefered_format_ == EF_Unknown)
    // {
    //     switch (slot_)
    //     {
    //     case RenderMaterial::TS_Albedo:
    //     case RenderMaterial::TS_Emissive:
    //         prefered_format_ = caps.BestMatchTextureFormat(MakeSpan({EF_BC7_SRGB, EF_BC1_SRGB, EF_ETC1}));
    //         break;

    //     case RenderMaterial::TS_MetalnessGlossiness:
    //     case RenderMaterial::TS_Normal:
    //         prefered_format_ = caps.BestMatchTextureFormat(MakeSpan({EF_BC5, EF_BC3, EF_GR8}));
    //         break;

    //     case RenderMaterial::TS_Height:
    //         prefered_format_ = caps.BestMatchTextureFormat(MakeSpan({EF_BC4, EF_BC1, EF_ETC1}));
    //         break;

    //     default:
    //         KFL_UNREACHABLE("Invalid texture slot");
    //     }
    // }
}

uint32_t TexMetadata::ArraySize() const
{
    return plane_file_names_.empty() ? 1 : static_cast<uint32_t>(plane_file_names_.size());
}

void TexMetadata::ArraySize(uint32_t size)
{
    plane_file_names_.resize(size);
}

std::string_view TexMetadata::PlaneFileName(uint32_t array_index, uint32_t mip) const
{
    if ((plane_file_names_.size() > array_index) && (plane_file_names_[array_index].size() > mip))
    {
        return plane_file_names_[array_index][mip];
    }
    else
    {
        return "";
    }
}

void TexMetadata::PlaneFileName(uint32_t array_index, uint32_t mip, std::string_view name)
{
    plane_file_names_[array_index][mip] = std::string(std::move(name));
}
}