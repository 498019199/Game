#include "D3D11FrameBuffer.h"
#include "D3D11RenderView.h"
#include "D3D11RenderEngine.h"

namespace RenderWorker
{
using namespace CommonWorker;

D3D11FrameBuffer::D3D11FrameBuffer()
{
    d3d_viewport_.MinDepth = 0.0f;
    d3d_viewport_.MaxDepth = 1.0f;
}

ID3D11RenderTargetView* D3D11FrameBuffer::D3DRTView(uint32_t n) const
{
    if (n < rt_views_.size())
    {
        if (rt_views_[n])
        {
            return checked_cast<D3D11RenderTargetView&>(*rt_views_[n]).RetrieveD3DRenderTargetView();
        }
    }

    return nullptr;
}

ID3D11DepthStencilView* D3D11FrameBuffer::D3DDSView() const
{
    if (ds_view_)
    {
        return checked_cast<D3D11DepthStencilView&>(*ds_view_).RetrieveD3DDepthStencilView();
    }

    return nullptr;
}

ID3D11UnorderedAccessView* D3D11FrameBuffer::D3DUAView(uint32_t n) const
{
    if (n < ua_views_.size())
    {
        if (ua_views_[n])
        {
            return checked_cast<D3D11UnorderedAccessView&>(*ua_views_[n]).RetrieveD3DUnorderedAccessView();
        }
    }

    return nullptr;
}

void D3D11FrameBuffer::OnBind()
{
    if (views_dirty_)
    {
        d3d_rt_src_.clear();
        d3d_rt_first_subres_.clear();
        d3d_rt_num_subres_.clear();
        d3d_rt_view_.resize(rt_views_.size());
        for (uint32_t i = 0; i < rt_views_.size(); ++ i)
        {
            if (rt_views_[i])
            {
                auto& p = checked_cast<D3D11RenderTargetView&>(*rt_views_[i]);
                d3d_rt_src_.push_back(p.RTSrc());
                d3d_rt_first_subres_.push_back(p.RTFirstSubRes());
                d3d_rt_num_subres_.push_back(p.RTNumSubRes());
                d3d_rt_view_[i] = this->D3DRTView(i);
            }
            else
            {
                d3d_rt_view_[i] = nullptr;
            }
        }
        if (ds_view_)
        {
            auto& p = checked_cast<D3D11DepthStencilView&>(*ds_view_);
            d3d_rt_src_.push_back(p.RTSrc());
            d3d_rt_first_subres_.push_back(p.RTFirstSubRes());
            d3d_rt_num_subres_.push_back(p.RTNumSubRes());
            d3d_ds_view_ = this->D3DDSView();
        }
        else
        {
            d3d_ds_view_ = nullptr;
        }

        d3d_ua_view_.resize(ua_views_.size());
        d3d_ua_init_count_.resize(ua_views_.size());
        for (uint32_t i = 0; i < ua_views_.size(); ++ i)
        {
            if (ua_views_[i])
            {
                auto& p = checked_cast<D3D11UnorderedAccessView&>(*ua_views_[i]);
                d3d_rt_src_.push_back(p.UASrc());
                d3d_rt_first_subres_.push_back(p.UAFirstSubRes());
                d3d_rt_num_subres_.push_back(p.UANumSubRes());
                d3d_ua_view_[i] = this->D3DUAView(i);
                d3d_ua_init_count_[i] = ua_views_[i]->InitCount();
            }
            else
            {
                d3d_ua_view_[i] = nullptr;
                d3d_ua_init_count_[i] = 0;
            }
        }

        d3d_viewport_.TopLeftX = static_cast<float>(viewport_->Left());
        d3d_viewport_.TopLeftY = static_cast<float>(viewport_->Top());
        d3d_viewport_.Width = static_cast<float>(viewport_->Width());
        d3d_viewport_.Height = static_cast<float>(viewport_->Height());

        views_dirty_ = false;
    }

    auto& d3d11_re = checked_cast<D3D11RenderEngine&>(Context::Instance().RenderEngineInstance());
    for (size_t i = 0; i < d3d_rt_src_.size(); ++ i)
    {
        d3d11_re.DetachSRV(d3d_rt_src_[i], d3d_rt_first_subres_[i], d3d_rt_num_subres_[i]);
    }

    if (ua_views_.empty())
    {
        //d3d11_re.OMSetRenderTargets(static_cast<UINT>(d3d_rt_view_.size()), d3d_rt_view_.data(), d3d_ds_view_);
    }
    else
    {
        // d3d11_re.OMSetRenderTargetsAndUnorderedAccessViews(static_cast<UINT>(d3d_rt_view_.size()), d3d_rt_view_.data(), d3d_ds_view_,
        //     static_cast<UINT>(d3d_rt_view_.size()), static_cast<UINT>(d3d_ua_view_.size()), d3d_ua_view_.data(),
        //     d3d_ua_init_count_.data());
    }

    d3d11_re.RSSetViewports(1, &d3d_viewport_);
}

void D3D11FrameBuffer::OnUnbind()
{
    
}

void D3D11FrameBuffer::Clear(uint32_t flags, Color const & clr, float depth, int32_t stencil)
{
    if (flags & CBM_Color)
    {
        for (uint32_t i = 0; i < rt_views_.size(); ++ i)
        {
            if (rt_views_[i])
            {
                rt_views_[i]->ClearColor(clr);
            }
        }
    }
    if ((flags & CBM_Depth) && (flags & CBM_Stencil))
    {
        if (ds_view_)
        {
            ds_view_->ClearDepthStencil(depth, stencil);
        }
    }
    else
    {
        if (flags & CBM_Depth)
        {
            if (ds_view_)
            {
                ds_view_->ClearDepth(depth);
            }
        }

        if (flags & CBM_Stencil)
        {
            if (ds_view_)
            {
                ds_view_->ClearStencil(stencil);
            }
        }
    }
}

void D3D11FrameBuffer::Discard(uint32_t flags)
{
    if (flags & CBM_Color)
    {
        for (uint32_t i = 0; i < rt_views_.size(); ++ i)
        {
            if (rt_views_[i])
            {
                rt_views_[i]->Discard();
            }
        }
    }
    if ((flags & CBM_Depth) || (flags & CBM_Stencil))
    {
        if (ds_view_)
        {
            ds_view_->Discard();
        }
    }
}
}