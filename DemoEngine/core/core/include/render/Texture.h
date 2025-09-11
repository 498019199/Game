
#pragma once
#include <render/ElementFormat.h>
#include <base/ZEngine.h>

namespace RenderWorker
{
#ifdef KLAYGE_SHIP
    #define KLAYGE_TEXTURE_DEBUG_NAME(texture)
#else
    #define KLAYGE_TEXTURE_DEBUG_NAME(texture) texture->DebugName(L ## #texture)
#endif
    
enum TextureMapAccess
{
    TMA_Read_Only,
    TMA_Write_Only,
    TMA_Read_Write
};

enum class TextureFilter : uint32_t
{
    Point,
    Linear,
};

class ZENGINE_CORE_API Texture: public std::enable_shared_from_this<Texture>
{
    ZENGINE_NONCOPYABLE(Texture);
public:
    // Enum identifying the texture type
    enum TextureType
    {
        // 1D texture, used in combination with 1D texture coordinates
        TT_1D,
        // 2D texture, used in combination with 2D texture coordinates
        TT_2D,
        // 3D texture, used in combination with 3D texture coordinates
        TT_3D,
        // cube map, used in combination with 3D texture coordinates
        TT_Cube
    };

    enum CubeFaces
    {
        CF_Positive_X = 0,
        CF_Negative_X = 1,
        CF_Positive_Y = 2,
        CF_Negative_Y = 3,
        CF_Positive_Z = 4,
        CF_Negative_Z = 5
    };
    
    Texture(TextureType type, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint);
    virtual ~Texture() noexcept;

#ifndef KLAYGE_SHIP
    virtual void DebugName([[maybe_unused]] std::wstring_view name)
    {
    }
#endif
    // Returns the texture type of the texture.
	TextureType Type() const;

	uint32_t SampleCount() const;
	uint32_t SampleQuality() const;

	uint32_t AccessHint() const;

    // Gets the number of mipmaps to be used for this texture.
	uint32_t MipMapsNum() const;
	// Gets the size of texture array
	uint32_t ArraySize() const;

    // Returns the pixel format for the texture surface.
	ElementFormat Format() const;

    // Returns the width of the texture.
    virtual uint32_t Width(uint32_t level) const = 0;
    // Returns the height of the texture.
    virtual uint32_t Height(uint32_t level) const = 0;
    // Returns the depth of the texture (only for 3D texture).
    virtual uint32_t Depth(uint32_t level) const = 0;


    virtual void CreateHWResource(std::span<ElementInitData const> init_data, float4 const * clear_value_hint) = 0;
    virtual void DeleteHWResource() = 0;
    virtual bool HWResourceReady() const = 0;
protected:
    uint32_t		mip_maps_num_;
    uint32_t		array_size_;

    ElementFormat	format_;
    TextureType		type_;
    uint32_t		sample_count_, sample_quality_;
    uint32_t		access_hint_;
};


using TexturePtr = std::shared_ptr<Texture>;

class ZENGINE_CORE_API VirtualTexture : public Texture
{
public:
    VirtualTexture(TextureType type, uint32_t width, uint32_t height, uint32_t depth, uint32_t num_mipmaps, uint32_t array_size,
        ElementFormat format, bool ref_only);

    uint32_t Width(uint32_t level) const override;
	uint32_t Height(uint32_t level) const override;
	uint32_t Depth(uint32_t level) const override;

    void CreateHWResource(std::span<ElementInitData const> init_data, float4 const * clear_value_hint) override;
    void DeleteHWResource() override;
    bool HWResourceReady() const override;

    const std::vector<ElementInitData>& SubresourceData() const
    {
        return subres_data_;
    }
    const std::vector<uint8_t>& DataBlock() const
    {
        return data_block_;
    }
private:
    bool ref_only_;

    uint32_t width_;
    uint32_t height_;
    uint32_t depth_;

    std::vector<ElementInitData> subres_data_;
    std::vector<uint8_t> data_block_;
    std::vector<std::unique_ptr<std::atomic<bool>>> mapped_;
};



void GetImageInfo(std::string_view tex_name, Texture::TextureType& type,
    uint32_t& width, uint32_t& height, uint32_t& depth, uint32_t& num_mipmaps, uint32_t& array_size,
    ElementFormat& format, uint32_t& row_pitch, uint32_t& slice_pitch);
    
TexturePtr LoadVirtualTexture(std::string_view tex_name);
TexturePtr SyncLoadTexture(std::string_view tex_name, uint32_t access_hint);
}