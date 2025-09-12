#pragma once
#include <base/DevHelper.h>
#include <string_view>
#include <base/ZEngine.h>

namespace RenderWorker
{
class TexMetadata;

class ImagePlane final
{
public:
    bool Load(std::string_view name, TexMetadata const & metadata);
    void RgbToLum();
    void AlphaToLum();
    void BumpToNormal(float scale, float amplitude);
    void NormalToHeight(float min_z);
    void PrepareNormalCompression(ElementFormat normal_compression_format);
    void FormatConversion(ElementFormat format);
    ImagePlane ResizeTo(uint32_t width, uint32_t height, bool linear);

    uint32_t Width() const
    {
        return uncompressed_tex_->Width(0);
    }
    uint32_t Height() const
    {
        return uncompressed_tex_->Height(0);
    }
    TexturePtr const & UncompressedTex() const
    {
        return uncompressed_tex_;
    }
    TexturePtr const & CompressedTex() const
    {
        return compressed_tex_;
    }

private:
	float RgbToLum(Color const & clr);

private:
    TexturePtr uncompressed_tex_;
    TexturePtr compressed_tex_;
};

}