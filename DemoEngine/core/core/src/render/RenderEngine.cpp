#include <base/ZEngine.h>
#include <base/App3D.h>
#include <base/Window.h>
#include <world/World.h>
#include <render/RenderEngine.h>
#include <render/RenderFactory.h>

#include <mutex>
namespace
{
	std::mutex default_mtl_instance_mutex;
	std::mutex mtl_cb_instance_mutex;
	std::mutex mesh_cb_instance_mutex;
	std::mutex model_cb_instance_mutex;
	std::mutex camera_cb_instance_mutex;
	std::mutex mipmapper_instance_mutex;
}

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
    //const RenderDeviceCaps& caps = DeviceCaps();

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
    this->Stereo(settings.stereo_method);
    this->StereoSeparation(settings.stereo_separation);
    this->DisplayOutput(settings.display_output_method);
    this->PaperWhiteNits(settings.paper_white);
    this->DisplayMaxLuminanceNits(settings.display_max_luminance);
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

const FrameBufferPtr& RenderEngine::OverlayFrameBuffer() const
{
	return overlay_frame_buffer_;
}

StereoMethod RenderEngine::Stereo() const
{
	return stereo_method_;
}

void RenderEngine::Stereo(StereoMethod method)
{
	stereo_method_ = method;		
    if (stereo_method_ != STM_None)
    {
        std::string pp_name;
        switch (stereo_method_)
        {
			case STM_ColorAnaglyph_RedCyan:
				pp_name = "stereoscopic_red_cyan";
				break;

			case STM_ColorAnaglyph_YellowBlue:
				pp_name = "stereoscopic_yellow_blue";
				break;

			case STM_ColorAnaglyph_GreenRed:
				pp_name = "stereoscopic_green_red";
				break;

			case STM_HorizontalInterlacing:
				pp_name = "stereoscopic_hor_interlacing";
				break;

			case STM_VerticalInterlacing:
				pp_name = "stereoscopic_ver_interlacing";
				break;

			case STM_Horizontal:
				pp_name = "stereoscopic_horizontal";
				break;

			case STM_Vertical:
				pp_name = "stereoscopic_vertical";
				break;

			case STM_LCDShutter:
				pp_name = "stereoscopic_lcd_shutter";
				break;

			case STM_OculusVR:
				pp_name = "stereoscopic_oculus_vr";
				break;
            default:
                ZENGINE_UNREACHABLE("Invalid stereo method");
        }

		//stereoscopic_pp_ = SyncLoadPostProcess("Stereoscopic.ppml", pp_name);
    }
}

void RenderEngine::StereoSeparation(float separation)
{
	stereo_separation_ = separation;
}

float RenderEngine::StereoSeparation() const
{
    return stereo_separation_;
}

DisplayOutputMethod RenderEngine::DisplayOutput() const
{
    return display_output_method_;
}

void RenderEngine::DisplayOutput(DisplayOutputMethod method)
{
    display_output_method_ = method;

    if (display_output_method_ != DOM_sRGB)
    {
        std::string pp_name;
        switch (display_output_method_)
        {
        case DOM_HDR10:
            pp_name = "DisplayHDR10";
            break;

        default:
            ZENGINE_UNREACHABLE("Invalid display output method");
        }
        //hdr_display_pp_ = SyncLoadPostProcess("HDRDisplay.ppml", pp_name);

        //hdr_enabled_ = true;
        //gamma_enabled_ = false;
        //color_grading_enabled_ = false;
    }
    else
    {
        //hdr_display_pp_.reset();
    }

    //pp_chain_dirty_ = true;
}

void RenderEngine::PaperWhiteNits(uint32_t nits)
{
    paper_white_ = nits;
    //this->UpdateHDRRescale();
}

uint32_t RenderEngine::PaperWhiteNits() const
{
    return paper_white_;
}

void RenderEngine::DisplayMaxLuminanceNits(uint32_t nits)
{
    display_max_luminance_ = nits;
    //this->UpdateHDRRescale();
}

uint32_t RenderEngine::DisplayMaxLuminanceNits() const
{
    return display_max_luminance_;
}

void RenderEngine::DestroyRenderWindow()
{
    if (cur_frame_buffer_)
    {
        cur_frame_buffer_->OnUnbind();
    }
    cur_frame_buffer_.reset();

    screen_frame_buffer_.reset();
    overlay_frame_buffer_.reset();

    for (int i = 3; i >= 0; -- i)
    {
        default_frame_buffers_[i].reset();
    }
}

void RenderEngine::Destroy()
{
    cur_frame_buffer_.reset();
    screen_frame_buffer_.reset();

    for (int i = 0; i < 4; ++ i)
    {
        default_frame_buffers_[i].reset();
    }

    overlay_frame_buffer_.reset();

	so_buffers_.reset();

    cur_rs_obj_.reset();
    cur_line_rs_obj_.reset();

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

PredefinedMaterialCBuffer const& RenderEngine::PredefinedMaterialCBufferInstance() const
{
    if (!predefined_material_cb_)
    {
        std::lock_guard<std::mutex> lock(mtl_cb_instance_mutex);
        if (!predefined_material_cb_)
        {
            predefined_material_cb_ = MakeUniquePtr<PredefinedMaterialCBuffer>();
        }
    }
    return *predefined_material_cb_;
}
}