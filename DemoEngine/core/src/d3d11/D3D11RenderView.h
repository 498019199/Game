#include <render/RenderView.h>
#include "D3D11Util.h"

namespace RenderWorker
{
class D3D11ShaderResourceView : public ShaderResourceView
{
public:
    virtual ID3D11ShaderResourceView* RetrieveD3DShaderResourceView() const = 0;

protected:
    ID3D11Device* d3d_device_;
    ID3D11DeviceContext* d3d_imm_ctx_;

    mutable ID3D11ShaderResourceViewPtr d3d_sr_view_;
    void* sr_src_;
};

class D3D11TextureShaderResourceView final : public D3D11ShaderResourceView
{
public:
    D3D11TextureShaderResourceView(TexturePtr const & texture, ElementFormat pf, uint32_t first_array_index, uint32_t array_size,
        uint32_t first_level, uint32_t num_levels);

    ID3D11ShaderResourceView* RetrieveD3DShaderResourceView() const override;
};
}