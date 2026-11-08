#pragma once

#include <render/GraphicsBuffer.h>
#include <render/ElementFormat.h>
#include "D3D11Util.h"

namespace RenderWorker
{

class D3D11GraphicsBuffer: public GraphicsBuffer
{
public:
	D3D11GraphicsBuffer(BufferUsage usage, uint32_t access_hint, uint32_t bind_flags,
			uint32_t size_in_byte, uint32_t structure_byte_stride);

    const ID3D11ShaderResourceViewPtr& RetrieveD3DShaderResourceView(ElementFormat pf, uint32_t first_elem, uint32_t num_elems);
    const ID3D11RenderTargetViewPtr& RetrieveD3DRenderTargetView(ElementFormat pf, uint32_t first_elem, uint32_t num_elems);
    const ID3D11UnorderedAccessViewPtr& RetrieveD3DUnorderedAccessView(ElementFormat pf, uint32_t first_elem, uint32_t num_elems);

    virtual void CopyToBuffer(GraphicsBuffer& target) override;
    virtual void CopyToSubBuffer(GraphicsBuffer& target, uint32_t dst_offset, uint32_t src_offset, uint32_t size) override;

    void CreateHWResource(void const * init_data) override;
    void DeleteHWResource() override;
    bool HWResourceReady() const override;

    void UpdateSubresource(uint32_t offset, uint32_t size, void const * data) override;

    void GetD3DFlags(D3D11_USAGE& usage, UINT& cpu_access_flags, UINT& bind_flags, UINT& misc_flags);

    ID3D11Buffer* D3DBuffer() const
    {
        return d3d_buffer_.get();
    }

private:
    void* Map(BufferAccess ba) override;
    void Unmap() override;

private:
    ID3D11Device* d3d_device_;
    ID3D11DeviceContext* d3d_imm_ctx_;

    ID3D11BufferPtr d3d_buffer_;
    uint32_t bind_flags_;

    // TODO: Not caching those views
    std::unordered_map<size_t, ID3D11ShaderResourceViewPtr> d3d_sr_views_;
    std::unordered_map<size_t, ID3D11RenderTargetViewPtr> d3d_rt_views_;
    std::unordered_map<size_t, ID3D11UnorderedAccessViewPtr> d3d_ua_views_;
};
}
