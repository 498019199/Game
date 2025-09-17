#include <render/RenderDeviceCaps.h>

namespace RenderWorker
{

bool RenderDeviceCaps::TextureFormatSupport(ElementFormat format) const
{
    auto iter = std::lower_bound(texture_formats_.begin(), texture_formats_.end(), format);
    return (iter != texture_formats_.end()) && (*iter == format);
}

void RenderDeviceCaps::AssignTextureFormats(std::vector<ElementFormat> texture_formats)
{
    std::sort(texture_formats.begin(), texture_formats.end());
    texture_formats.erase(std::unique(texture_formats.begin(), texture_formats.end()), texture_formats.end());

    texture_formats_ = std::move(texture_formats);

    this->UpdateSupportBits();
}

ElementFormat RenderDeviceCaps::BestMatchTextureFormat(std::span<const ElementFormat> formats) const
{
    ElementFormat ret = EF_Unknown;
    for (auto fmt : formats)
    {
        if (this->TextureFormatSupport(fmt))
        {
            ret = fmt;
            break;
        }
    }

    return ret;
}

void RenderDeviceCaps::UpdateSupportBits()
{
    
}

}