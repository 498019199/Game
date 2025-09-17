#include "D3D11RenderView.h"

#include <base/ZEngine.h>
#include <render/RenderFactory.h>

#include "D3D11RenderEngine.h"
#include "D3D11Texture.h"
#include "D3D11GraphicsBuffer.h"

namespace RenderWorker
{
using namespace CommonWorker;

D3D11TextureShaderResourceView::D3D11TextureShaderResourceView(TexturePtr const & texture, ElementFormat pf, uint32_t first_array_index, uint32_t array_size,
        uint32_t first_level, uint32_t num_levels)
{
    COMMON_ASSERT(texture->AccessHint() & EAH_GPU_Read);

    const auto& d3d11_re = checked_cast<const D3D11RenderEngine&>(
        Context::Instance().RenderFactoryInstance().RenderEngineInstance());
    d3d_device_ = d3d11_re.D3DDevice1();
    d3d_imm_ctx_ = d3d11_re.D3DDeviceImmContext1();

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

#if ZENGINE_IS_DEV_PLATFORM
void* D3D11TextureShaderResourceView::GetShaderResourceView()
{
	return RetrieveD3DShaderResourceView();
}
#endif//ZENGINE_IS_DEV_PLATFORM




D3D11RenderTargetView::D3D11RenderTargetView(void* src, uint32_t first_subres, uint32_t num_subres)
    : rt_src_(src), rt_first_subres_(first_subres), rt_num_subres_(num_subres)
{
    const auto& d3d11_re = checked_cast<const D3D11RenderEngine&>(
        Context::Instance().RenderFactoryInstance().RenderEngineInstance());
    d3d_device_ = d3d11_re.D3DDevice1();
    d3d_imm_ctx_ = d3d11_re.D3DDeviceImmContext1();
}

void D3D11RenderTargetView::ClearColor(Color const & clr)
{
    // 清除目标视图
    d3d_imm_ctx_->ClearRenderTargetView(d3d_rt_view_.get(), &clr.r());
}

void D3D11RenderTargetView::Discard()
{
    // 放弃视图
    d3d_imm_ctx_->DiscardView(d3d_rt_view_.get());
}

void D3D11RenderTargetView::OnAttached([[maybe_unused]] FrameBuffer& fb, FrameBuffer::Attachment att)
{
}

void D3D11RenderTargetView::OnDetached([[maybe_unused]] FrameBuffer& fb, FrameBuffer::Attachment att)
{
}

D3D11Texture1D2DCubeRenderTargetView::D3D11Texture1D2DCubeRenderTargetView(const TexturePtr& texture, 
        ElementFormat pf, int first_array_index, int array_size, int level)
		: D3D11RenderTargetView(texture.get(), first_array_index * texture->MipMapsNum() + level, 1)
{
    COMMON_ASSERT(texture);

    tex_ = texture;
    width_ = texture->Width(level);
    height_ = texture->Height(level);
    pf_ = pf == EF_Unknown ? texture->Format() : pf;
    sample_count_ = texture->SampleCount();
    sample_quality_ = texture->SampleQuality();

    first_array_index_ = first_array_index;
    array_size_ = array_size;
    level_ = level;
    first_slice_ = 0;
    num_slices_ = texture->Depth(0);
    first_face_ = Texture::CF_Positive_X;
    num_faces_ = 1;
    first_elem_ = 0;
    num_elems_ = 0;

    this->RetrieveD3DRenderTargetView();
}

ID3D11RenderTargetView* D3D11Texture1D2DCubeRenderTargetView::RetrieveD3DRenderTargetView() const 
{
    if (!d3d_rt_view_ && tex_->HWResourceReady())
    {
        d3d_rt_view_ = checked_cast<D3D11Texture&>(*tex_).RetrieveD3DRenderTargetView(pf_, first_array_index_, array_size_, level_);
    }
    return d3d_rt_view_.get();
}




D3D11Texture3DRenderTargetView::D3D11Texture3DRenderTargetView(TexturePtr const & texture_3d, ElementFormat pf, 
        int array_index, uint32_t first_slice, uint32_t num_slices, int level)
    :D3D11RenderTargetView(texture_3d.get(),
			(array_index * texture_3d->Depth(level) + first_slice) * texture_3d->MipMapsNum() + level,
			num_slices * texture_3d->MipMapsNum() + level)
{
    COMMON_ASSERT(texture_3d);

    tex_ = texture_3d;
    width_ = texture_3d->Width(level);
    height_ = texture_3d->Height(level);
    pf_ = pf == EF_Unknown ? texture_3d->Format() : pf;
    sample_count_ = texture_3d->SampleCount();
    sample_quality_ = texture_3d->SampleQuality();

    first_array_index_ = array_index;
    array_size_ = 1;
    level_ = level;
    first_slice_ = first_slice;
    num_slices_ = num_slices;
    first_face_ = Texture::CF_Positive_X;
    num_faces_ = 1;
    first_elem_ = 0;
    num_elems_ = 0;

    this->RetrieveD3DRenderTargetView();
}

ID3D11RenderTargetView* D3D11Texture3DRenderTargetView::RetrieveD3DRenderTargetView() const 
{
    if (!d3d_rt_view_ && tex_->HWResourceReady())
    {
        d3d_rt_view_ = checked_cast<D3D11Texture&>(*tex_).RetrieveD3DRenderTargetView(pf_, first_array_index_, first_slice_,
            num_slices_, level_);
    }
    return d3d_rt_view_.get();
}




D3D11TextureCubeFaceRenderTargetView::D3D11TextureCubeFaceRenderTargetView(TexturePtr const & texture_cube, ElementFormat pf, 
        int array_index, Texture::CubeFaces face, int level)
    :D3D11RenderTargetView(texture_cube.get(), (array_index * 6 + face) * texture_cube->MipMapsNum() + level, 1)
{
    COMMON_ASSERT(texture_cube);

    tex_ = texture_cube;
    width_ = texture_cube->Width(level);
    height_ = texture_cube->Width(level);
    pf_ = pf == EF_Unknown ? texture_cube->Format() : pf;
    sample_count_ = texture_cube->SampleCount();
    sample_quality_ = texture_cube->SampleQuality();

    first_array_index_ = array_index;
    array_size_ = 1;
    level_ = level;
    first_slice_ = 0;
    num_slices_ = texture_cube->Depth(0);
    first_face_ = face;
    num_faces_ = 1;
    first_elem_ = 0;
    num_elems_ = 0;

    this->RetrieveD3DRenderTargetView();
}

ID3D11RenderTargetView* D3D11TextureCubeFaceRenderTargetView::RetrieveD3DRenderTargetView() const 
{
    if (!d3d_rt_view_ && tex_->HWResourceReady())
    {
        d3d_rt_view_ = checked_cast<D3D11Texture&>(*tex_).RetrieveD3DRenderTargetView(pf_, first_array_index_, first_face_,
            level_);
    }
    return d3d_rt_view_.get();
}




D3D11BufferRenderTargetView::D3D11BufferRenderTargetView(GraphicsBufferPtr const & gb, ElementFormat pf, 
        uint32_t first_elem, uint32_t num_elems)
    :D3D11RenderTargetView(gb.get(), 0, 1)
{
    COMMON_ASSERT(gb);
    COMMON_ASSERT(gb->AccessHint() & EAH_GPU_Write);

    buff_ = gb;
    width_ = num_elems;
    height_ = 1;
    pf_ = pf;
    sample_count_ = 1;
    sample_quality_ = 0;

    first_array_index_ = 0;
    array_size_ = 0;
    level_ = 0;
    first_slice_ = 0;
    num_slices_ = 0;
    first_face_ = Texture::CF_Positive_X;
    num_faces_ = 1;
    first_elem_ = first_elem;
    num_elems_ = num_elems;

    this->RetrieveD3DRenderTargetView();
}

ID3D11RenderTargetView* D3D11BufferRenderTargetView::RetrieveD3DRenderTargetView() const 
{
    if (!d3d_rt_view_ && buff_->HWResourceReady())
    {
        d3d_rt_view_ = checked_cast<D3D11GraphicsBuffer&>(*buff_).RetrieveD3DRenderTargetView(pf_, first_elem_, num_elems_);
    }
    return d3d_rt_view_.get();
}






D3D11DepthStencilView::D3D11DepthStencilView(void* src, uint32_t first_subres, uint32_t num_subres)
{
    const auto& d3d11_re = checked_cast<const D3D11RenderEngine&>(
        Context::Instance().RenderFactoryInstance().RenderEngineInstance());
    d3d_device_ = d3d11_re.D3DDevice1();
    d3d_imm_ctx_ = d3d11_re.D3DDeviceImmContext1();
}

void D3D11DepthStencilView::ClearDepth(float depth)
{
    d3d_imm_ctx_->ClearDepthStencilView(d3d_ds_view_.get(), D3D11_CLEAR_DEPTH, depth, 0);
}

void D3D11DepthStencilView::ClearStencil(int32_t stencil)
{
    d3d_imm_ctx_->ClearDepthStencilView(d3d_ds_view_.get(), D3D11_CLEAR_STENCIL, 1, static_cast<uint8_t>(stencil));
}

void D3D11DepthStencilView::ClearDepthStencil(float depth, int32_t stencil)
{
    d3d_imm_ctx_->ClearDepthStencilView(d3d_ds_view_.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth,
        static_cast<uint8_t>(stencil));
}

void D3D11DepthStencilView::Discard()
{
    d3d_imm_ctx_->DiscardView(d3d_ds_view_.get());
}

void D3D11DepthStencilView::OnAttached([[maybe_unused]]  FrameBuffer& fb)
{
    
}

void D3D11DepthStencilView::OnDetached([[maybe_unused]] FrameBuffer& fb)
{
    
}

D3D11Texture1D2DCubeDepthStencilView::D3D11Texture1D2DCubeDepthStencilView(const TexturePtr& texture, ElementFormat pf, int first_array_index, int array_size,
        int level)
    :D3D11DepthStencilView(texture.get(), first_array_index * texture->MipMapsNum() + level, 1)
{
    COMMON_ASSERT(texture);

    tex_ = texture;
    width_ = texture->Width(level);
    height_ = texture->Height(level);
    pf_ = pf == EF_Unknown ? texture->Format() : pf;
    sample_count_ = texture->SampleCount();
    sample_quality_ = texture->SampleQuality();

    first_array_index_ = first_array_index;
    array_size_ = array_size;
    level_ = level;
    first_slice_ = 0;
    num_slices_ = texture->Depth(0);
    first_face_ = Texture::CF_Positive_X;
    num_faces_ = 1;

    this->RetrieveD3DDepthStencilView();
}

D3D11Texture1D2DCubeDepthStencilView::D3D11Texture1D2DCubeDepthStencilView(uint32_t width, uint32_t height, ElementFormat pf, uint32_t sample_count,
        uint32_t sample_quality)
    :D3D11DepthStencilView(nullptr, 0, 1)
{
    COMMON_ASSERT(IsDepthFormat(pf));

    auto& rf = Context::Instance().RenderFactoryInstance();
    tex_ = rf.MakeTexture2D(width, height, 1, 1, pf, sample_count, sample_quality, EAH_GPU_Write);
    rt_src_ = tex_.get();

    width_ = width;
    height_ = height;
    pf_ = pf;
    sample_count_ = sample_count;
    sample_quality_ = sample_quality;

    first_array_index_ = 0;
    array_size_ = 1;
    level_ = 0;
    first_slice_ = 0;
    num_slices_ = tex_->Depth(0);
    first_face_ = Texture::CF_Positive_X;
    num_faces_ = 1;

    this->RetrieveD3DDepthStencilView();
}

ID3D11DepthStencilView* D3D11Texture1D2DCubeDepthStencilView::RetrieveD3DDepthStencilView() const 
{
    if (!d3d_ds_view_ && tex_->HWResourceReady())
    {
        d3d_ds_view_ = checked_cast<D3D11Texture&>(*tex_).RetrieveD3DDepthStencilView(pf_, first_array_index_, array_size_,
            level_);
    }
    return d3d_ds_view_.get();
}




D3D11Texture3DDepthStencilView::D3D11Texture3DDepthStencilView(TexturePtr const & texture_3d, ElementFormat pf, int array_index, uint32_t first_slice,
        uint32_t num_slices, int level)
    : D3D11DepthStencilView(texture_3d.get(), (array_index * texture_3d->Depth(level) + first_slice) * texture_3d->MipMapsNum() + level,
		num_slices * texture_3d->MipMapsNum() + level)
{
    COMMON_ASSERT(texture_3d);

    tex_ = texture_3d;
    width_ = texture_3d->Width(level);
    height_ = texture_3d->Height(level);
    pf_ = pf == EF_Unknown ? texture_3d->Format() : pf;
    sample_count_ = texture_3d->SampleCount();
    sample_quality_ = texture_3d->SampleQuality();

    first_array_index_ = array_index;
    array_size_ = 1;
    level_ = level;
    first_slice_ = first_slice;
    num_slices_ = num_slices;
    first_face_ = Texture::CF_Positive_X;
    num_faces_ = 1;

    this->RetrieveD3DDepthStencilView();
}

ID3D11DepthStencilView* D3D11Texture3DDepthStencilView::RetrieveD3DDepthStencilView() const 
{
    if (!d3d_ds_view_ && tex_->HWResourceReady())
    {
        d3d_ds_view_ = checked_cast<D3D11Texture&>(*tex_).RetrieveD3DDepthStencilView(pf_, first_array_index_, first_slice_,
            num_slices_, level_);
    }
    return d3d_ds_view_.get();
}




D3D11TextureCubeFaceDepthStencilView::D3D11TextureCubeFaceDepthStencilView(TexturePtr const & texture_cube, ElementFormat pf, int array_index, Texture::CubeFaces face,
        int level)
    :D3D11DepthStencilView(texture_cube.get(), (array_index * 6 + face) * texture_cube->MipMapsNum() + level, 1)
{
    COMMON_ASSERT(texture_cube);

    tex_ = texture_cube;
    width_ = texture_cube->Width(level);
    height_ = texture_cube->Width(level);
    pf_ = pf == EF_Unknown ? texture_cube->Format() : pf;
    sample_count_ = texture_cube->SampleCount();
    sample_quality_ = texture_cube->SampleQuality();

    first_array_index_ = array_index;
    array_size_ = 1;
    level_ = level;
    first_slice_ = 0;
    num_slices_ = texture_cube->Depth(0);
    first_face_ = face;
    num_faces_ = 1;

    this->RetrieveD3DDepthStencilView();
}

ID3D11DepthStencilView* D3D11TextureCubeFaceDepthStencilView::RetrieveD3DDepthStencilView() const 
{
    if (!d3d_ds_view_ && tex_->HWResourceReady())
    {
        d3d_ds_view_ = checked_cast<D3D11Texture&>(*tex_).RetrieveD3DDepthStencilView(pf_, first_array_index_, first_face_, level_);
    }
    return d3d_ds_view_.get();
}






D3D11UnorderedAccessView::D3D11UnorderedAccessView(void* src, uint32_t first_subres, uint32_t num_subres)
    :ua_src_(src), ua_first_subres_(first_subres), ua_num_subres_(num_subres)
{
    const auto& d3d11_re = checked_cast<const D3D11RenderEngine&>(
        Context::Instance().RenderFactoryInstance().RenderEngineInstance());
    d3d_device_ = d3d11_re.D3DDevice1();
    d3d_imm_ctx_ = d3d11_re.D3DDeviceImmContext1();
}

void D3D11UnorderedAccessView::Clear(float4 const & val)
{
	d3d_imm_ctx_->ClearUnorderedAccessViewFloat(d3d_ua_view_.get(), &val.x());
}

void D3D11UnorderedAccessView::Clear(uint4 const & val)
{
    d3d_imm_ctx_->ClearUnorderedAccessViewUint(d3d_ua_view_.get(), &val.x());
}

void D3D11UnorderedAccessView::Discard()
{
    d3d_imm_ctx_->DiscardView(d3d_ua_view_.get());
}

void D3D11UnorderedAccessView::OnAttached([[maybe_unused]] FrameBuffer& fb, uint32_t index)
{
    
}

void D3D11UnorderedAccessView::OnDetached([[maybe_unused]] FrameBuffer& fb, uint32_t index)
{
    
}

}
