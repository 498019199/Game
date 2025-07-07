#include <render/RenderStateObject.h>
#include "D3D11Util.h"

namespace RenderWorker
{
class D3D11SamplerStateObject: public SamplerStateObject
{
public:
    explicit D3D11SamplerStateObject(SamplerStateDesc const & desc);

    ID3D11SamplerState* D3DSamplerState() const
    {
        return sampler_state_.get();
    }

private:
    ID3D11SamplerStatePtr sampler_state_;
};

class D3D11RenderStateObject final : public RenderStateObject
{
public: 
    D3D11RenderStateObject(const RasterizerStateDesc& rs_desc, const DepthStencilStateDesc& dss_desc, const BlendStateDesc& bs_desc);

    void Active() override;

    ID3D11RasterizerState* D3DRasterizerState() const
    {
        return rasterizer_state_.get();
    }

    ID3D11DepthStencilState* D3DDepthStencilState() const
    {
        return depth_stencil_state_.get();
    }

    ID3D11BlendState* D3DBlendState() const
    {
        return blend_state_.get();
    }
private:
    ID3D11RasterizerStatePtr rasterizer_state_;
    ID3D11DepthStencilStatePtr depth_stencil_state_;
    ID3D11BlendStatePtr blend_state_;
};
}
