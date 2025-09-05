#include <base/Context.h>
#include <base/App3D.h>
#include <base/Window.h>
#include <world/World.h>
#include <render/RenderEngine.h>
#include <render/RenderFactory.h>

namespace RenderWorker
{
RenderEngine::RenderEngine()
{
    
}

RenderEngine::~RenderEngine() noexcept = default;

#if ZENGINE_IS_DEV_PLATFORM
void* RenderEngine::GetD3DDevice()
{
    return nullptr;
}

void* RenderEngine::GetD3DDeviceImmContext()
{
    return nullptr;
}
#endif

void RenderEngine::CreateRenderWindow(std::string const & name, RenderSettings& settings)
{
    if (settings.stereo_method != STM_OculusVR)
    {
        stereo_separation_ = settings.stereo_separation;
    }

    DoCreateRenderWindow(name, settings);
    const RenderDeviceCaps& caps = DeviceCaps();

    screen_frame_buffer_ = cur_frame_buffer_;

    const uint32_t screen_width = screen_frame_buffer_->Width();
    const uint32_t screen_height = screen_frame_buffer_->Height();
    float const screen_aspect = static_cast<float>(screen_width) / screen_height;
    if (!MathWorker::equal(screen_aspect, static_cast<float>(settings.width) / settings.height))
    {
        settings.width = static_cast<uint32_t>(settings.height * screen_aspect + 0.5f);
    }

    for (int i = 0; i < 4; ++ i)
    {
        default_frame_buffers_[i] = screen_frame_buffer_;
    }

    this->BindFrameBuffer(default_frame_buffers_[0]);
}

void RenderEngine::BeginFrame()
{
    this->BindFrameBuffer(default_frame_buffers_[0]);
}

void RenderEngine::BeginPass()
{
    
}

void RenderEngine::Render(const RenderEffect& effect, const RenderTechnique& tech, const RenderLayout& rl)
{
    if (tech.HWResourceReady(effect))
    {
        this->DoRender(effect, tech, rl);
    }
}

void RenderEngine::EndPass()
{
	
}

void RenderEngine::EndFrame()
{
    
}

void RenderEngine::BindFrameBuffer(const FrameBufferPtr& fb)
{
    FrameBufferPtr new_fb;
    if (fb)
    {
        new_fb = fb;
    }
    else
    {
        new_fb = this->DefaultFrameBuffer();
    }

    if ((cur_frame_buffer_ != new_fb) || (new_fb && new_fb->Dirty()))
    {
        if (cur_frame_buffer_)
        {
            cur_frame_buffer_->OnUnbind();
        }

        cur_frame_buffer_ = new_fb;
        cur_frame_buffer_->OnBind();

        this->DoBindFrameBuffer(cur_frame_buffer_);
    }
}

const FrameBufferPtr& RenderEngine::CurFrameBuffer() const
{
    return cur_frame_buffer_;
}

const FrameBufferPtr& RenderEngine::DefaultFrameBuffer() const
{
    return default_frame_buffers_[fb_stage_];
}

const FrameBufferPtr& RenderEngine::ScreenFrameBuffer() const
{
    return screen_frame_buffer_;
}

void RenderEngine::DestroyRenderWindow()
{
    if (cur_frame_buffer_)
    {
        cur_frame_buffer_->OnUnbind();
    }
    cur_frame_buffer_.reset();

    for (int i = 3; i >= 0; -- i)
    {
        default_frame_buffers_[i].reset();
    }
}

void RenderEngine::Destroy()
{
    this->DoDestroy();
}

void RenderEngine::ForceLineMode(bool line)
{
    if (force_line_mode_ != line)
    {
        force_line_mode_ = line;

        if (cur_rs_obj_)
        {
            if (force_line_mode_)
            {
                auto rs_desc = cur_rs_obj_->GetRasterizerStateDesc();
                auto const & dss_desc = cur_rs_obj_->GetDepthStencilStateDesc();
                auto const & bs_desc = cur_rs_obj_->GetBlendStateDesc();
                rs_desc.polygon_mode = PM_Line;
                cur_line_rs_obj_ = Context::Instance().RenderFactoryInstance().MakeRenderStateObject(rs_desc, dss_desc, bs_desc);
                cur_line_rs_obj_->Active();
            }
            else
            {
                cur_rs_obj_->Active();
            }
        }
    }
}

// 设置当前渲染状态对象
/////////////////////////////////////////////////////////////////////////////////
void RenderEngine::SetStateObject(const RenderStateObjectPtr  & rs_obj)
{
    if (cur_rs_obj_ != rs_obj)
    {
        if (force_line_mode_)
        {
            auto rs_desc = rs_obj->GetRasterizerStateDesc();
            auto const & dss_desc = rs_obj->GetDepthStencilStateDesc();
            auto const & bs_desc = rs_obj->GetBlendStateDesc();
            rs_desc.polygon_mode = PM_Line;
            cur_line_rs_obj_ = Context::Instance().RenderFactoryInstance().MakeRenderStateObject(rs_desc, dss_desc, bs_desc);
            cur_line_rs_obj_->Active();
        }
        else
        {
            rs_obj->Active();
        }
        cur_rs_obj_ = rs_obj;
    }
}

const RenderDeviceCaps& RenderEngine::DeviceCaps() const
{
	return caps_;
}

void RenderEngine::BindSOBuffers(const RenderLayoutPtr& rl)
{
    so_buffers_ = rl;
    DoBindSOBuffers(rl);
}

void RenderEngine::Refresh() const
{
    auto& context = Context::Instance();
    if (context.AppInstance().MainWnd()->Active())
    {
        context.WorldInstance().Update();
    }
}

}