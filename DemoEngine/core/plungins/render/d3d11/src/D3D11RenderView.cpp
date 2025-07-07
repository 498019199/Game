#include "D3D11RenderView.h"

#include <base/Context.h>

#include "D3D11RenderEngine.h"
#include "D3D11Texture.h"

namespace RenderWorker
{
D3D11TextureShaderResourceView::D3D11TextureShaderResourceView(TexturePtr const & texture, ElementFormat pf, uint32_t first_array_index, uint32_t array_size,
        uint32_t first_level, uint32_t num_levels)
{
    COMMON_ASSERT(texture->AccessHint() & EAH_GPU_Read);

    auto const& re = checked_cast<const D3D11RenderEngine&>(Context::Instance().RenderEngineInstance());
    d3d_device_ = re.D3DDevice();
    d3d_imm_ctx_ = re.D3DDeviceImmContext();

    tex_ = texture;
    pf_ = pf == EF_Unknown ? texture->Format() : pf;

    first_array_index_ = first_array_index;
    array_size_ = array_size;
    first_level_ = first_level;
    num_levels_ = num_levels;
    first_elem_ = 0;
    num_elems_ = 0;

    sr_src_ = texture.get();
}

ID3D11ShaderResourceView* D3D11TextureShaderResourceView::RetrieveD3DShaderResourceView() const 
{
    if (!d3d_sr_view_ && tex_ && tex_->HWResourceReady())
    {
        d3d_sr_view_ = checked_cast<D3D11Texture&>(*tex_).RetrieveD3DShaderResourceView(pf_, first_array_index_, array_size_,
            first_level_, num_levels_);
    }
    return d3d_sr_view_.get();
}
}