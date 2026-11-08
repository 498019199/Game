#include <render/FrameBuffer.h>
#include <base/ZEngine.h>
#include <render/RenderFactory.h>
#include <render/RenderEngine.h>

namespace RenderWorker
{
FrameBuffer::FrameBuffer()
    :left_(0), top_(0), width_(0), height_(0), 
        viewport_(CommonWorker::MakeSharedPtr<RenderWorker::Viewport>())
{
    viewport_->Left(left_);
    viewport_->Top(top_);
    viewport_->Width(width_);
    viewport_->Height(height_);
}

FrameBuffer::~FrameBuffer() noexcept  = default;

const ViewportPtr& FrameBuffer::Viewport() const
{
    return viewport_;
}

ViewportPtr& FrameBuffer::Viewport()
{
    return viewport_;
}

void FrameBuffer::Viewport(const ViewportPtr& viewport)
{
    viewport_ = viewport;
}

void FrameBuffer::Attach(Attachment att, const RenderTargetViewPtr& view)
{
    auto& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
    if (std::to_underlying(att) >= re.DeviceCaps().max_simultaneous_rts)
    {
        TERRC(std::errc::function_not_supported);
    }

    const uint32_t rt_index = std::to_underlying(att);
    if ((rt_index < rt_views_.size()) && rt_views_[rt_index])
    {
        this->Detach(att);
    }

    if (rt_views_.size() < rt_index + 1)
    {
        rt_views_.resize(rt_index + 1);
    }

    rt_views_[rt_index] = view;
    size_t min_rt_index = rt_index;
    for (size_t i = 0; i < rt_index; ++ i)
    {
        if (rt_views_[i])
        {
            min_rt_index = i;
        }
    }
    if (min_rt_index == rt_index)
    {
        width_ = view->Width();
        height_ = view->Height();

        viewport_->Left(0);
        viewport_->Top(0);
        viewport_->Width(width_);
        viewport_->Height(height_);
    }

    if (view)
    {
        view->OnAttached(*this, att);
    }

    views_dirty_ = true;
}

void FrameBuffer::Detach(Attachment att)
{
    auto& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
    if (std::to_underlying(att) >= re.DeviceCaps().max_simultaneous_rts)
    {
        TERRC(std::errc::function_not_supported);
    }

    const uint32_t rt_index = std::to_underlying(att);
    if ((rt_index < rt_views_.size()) && rt_views_[rt_index])
    {
        rt_views_[rt_index]->OnDetached(*this, att);
        rt_views_[rt_index].reset();
    }

    views_dirty_ = true;
}

const RenderTargetViewPtr& FrameBuffer::AttachedRtv(Attachment att) const
{
    const uint32_t rt_index = std::to_underlying(att);
    if (rt_index < rt_views_.size())
    {
        return rt_views_[rt_index];
    }
    else
    {
        static RenderTargetViewPtr null_view;
        return null_view;
    }
}

void FrameBuffer::Attach(const DepthStencilViewPtr& view)
{
    if (ds_view_)
    {
        this->Detach();
    }

    ds_view_ = view;

    if (view)
    {
        view->OnAttached(*this);
    }

    views_dirty_ = true;
}

void FrameBuffer::Detach()
{
    ds_view_.reset();
    views_dirty_ = true;
}

const DepthStencilViewPtr& FrameBuffer::AttachedDsv() const
{
	return ds_view_;
}

void FrameBuffer::Attach(uint32_t index, const UnorderedAccessViewPtr& view)
{
    auto& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
    if (index >= re.DeviceCaps().max_simultaneous_uavs)
    {
        TERRC(std::errc::function_not_supported);
    }

    if ((index < ua_views_.size()) && ua_views_[index])
    {
        this->Detach(index);
    }

    if (ua_views_.size() < index + 1)
    {
        ua_views_.resize(index + 1);
    }

    ua_views_[index] = view;
    if (view)
    {
        view->OnAttached(*this, index);
    }

    views_dirty_ = true;
}

void FrameBuffer::Detach(uint32_t index)
{
    auto& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
    if (index >= re.DeviceCaps().max_simultaneous_rts)
    {
        TERRC(std::errc::function_not_supported);
    }

    if ((index < ua_views_.size()) && ua_views_[index])
    {
        ua_views_[index]->OnDetached(*this, index);
        ua_views_[index].reset();
    }

    views_dirty_ = true;
}

const UnorderedAccessViewPtr& FrameBuffer::AttachedUav(uint32_t index) const
{
    if (index < ua_views_.size())
    {
        return ua_views_[index];
    }
    else
    {
        static UnorderedAccessViewPtr null_view;
        return null_view;
    }
}

uint32_t FrameBuffer::Left() const
{
	return left_;
}

uint32_t FrameBuffer::Top() const
{
    return top_;
}

uint32_t FrameBuffer::Width() const
{
    return width_;
}

uint32_t FrameBuffer::Height() const
{
    return height_;
}
    
}