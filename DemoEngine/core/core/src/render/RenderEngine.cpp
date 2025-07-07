#include <Base/Context.h>

#include <Render/RenderEngine.h>

namespace RenderWorker
{
RenderEngine::RenderEngine()
{
    
}

RenderEngine::~RenderEngine() noexcept = default;

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
}