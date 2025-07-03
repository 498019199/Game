#pragma once
#include <render/Texture.h>

#include "D3D11Util.h"
#include <unordered_map>

namespace RenderWorker
{
class D3D11Texture: public Texture
{
public:
    D3D11Texture(TextureType type, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint);

#ifndef KLAYGE_SHIP
	void DebugName(std::wstring_view name) override;
#endif
    ID3D11Resource* D3DResource() const noexcept;
    void DeleteHWResource() override;
    bool HWResourceReady() const override;
    
    uint32_t Width(uint32_t level) const noexcept override;
    uint32_t Height(uint32_t level) const noexcept override;
    uint32_t Depth(uint32_t level) const noexcept override;

    ID3D11ShaderResourceViewPtr const & RetrieveD3DShaderResourceView(ElementFormat pf, uint32_t first_array_index, uint32_t array_size,
        uint32_t first_level, uint32_t num_levels);
protected:
    void GetD3DFlags(D3D11_USAGE& usage, UINT& bind_flags, UINT& cpu_access_flags, UINT& misc_flags);

    virtual D3D11_SHADER_RESOURCE_VIEW_DESC FillSRVDesc(
        ElementFormat pf, uint32_t first_array_index, uint32_t array_size, uint32_t first_level, uint32_t num_levels) const;
protected:
    ID3D11Device*				d3d_device_;
    ID3D11DeviceContext*		d3d_imm_ctx_;
    DXGI_FORMAT dxgi_fmt_;

    ID3D11ResourcePtr d3d_texture_;

    // TODO: Not caching those views
	std::unordered_map<size_t, ID3D11ShaderResourceViewPtr> d3d_sr_views_;
};


class D3D11Texture2D final : public D3D11Texture
{
public:
    D3D11Texture2D(uint32_t width, uint32_t height, uint32_t num_mip_maps, uint32_t array_size, ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint);
    explicit D3D11Texture2D(ID3D11Texture2DPtr const & d3d_tex);

    uint32_t Width(uint32_t level) const noexcept override;
    uint32_t Height(uint32_t level) const noexcept override;

    void CreateHWResource(std::span<ElementInitData const> init_data, float4 const * clear_value_hint) override;

protected:
    D3D11_SHADER_RESOURCE_VIEW_DESC FillSRVDesc(
        ElementFormat pf, uint32_t first_array_index, uint32_t array_size, uint32_t first_level, uint32_t num_levels) const override;
private:
    uint32_t width_;
    uint32_t height_;
};
}