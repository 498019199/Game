#include <base/ZEngine.h>
#include <base/App3D.h>
#include <world/World.h>

#include <render/RenderEffect.h>
#include <render/Renderable.h>
#include <render/RenderEngine.h>
#include <render/RenderFactory.h>
#include <render/FrameBuffer.h>

namespace RenderWorker
{

World::World()
    :scene_root_(L"SceenRoot", SceneNode::SOA_Cullable)
{
}

World::~World()
{

}

void World::AddRenderable(Renderable* obj)
{
    if(nullptr == obj)
    {
        return;
    }

    const RenderTechnique* obj_tech = obj->GetRenderTechnique();
    bool found = false;
    for (auto& items : render_queue_)
    {
        if (items.first == obj_tech)
        {
            items.second.push_back(obj);
            found = true;
            break;
        }
    }
    if (!found)
    {
        render_queue_.emplace_back(obj_tech, std::vector<Renderable*>(1, obj));
    }
}

void World::Flush(uint32_t urt)
{
    std::sort(render_queue_.begin(), render_queue_.end(),
    [](std::pair<const RenderTechnique*, std::vector<Renderable*>> const & lhs,
        std::pair<const RenderTechnique*, std::vector<Renderable*>> const & rhs)
    {
        COMMON_ASSERT(lhs.first);
        COMMON_ASSERT(rhs.first);

        return lhs.first->Weight() < rhs.first->Weight();
    });

    for (auto& items : render_queue_)
    {
        for (auto const & item : items.second)
        {
            item->Render();
        }
        num_renderables_rendered_ += static_cast<uint32_t>(items.second.size());
    }
}

void World::Update()
{
	auto& context = Context::Instance();
    auto& app = context.AppInstance();
    //const float app_time = app.AppTime();
    //const float frame_time = app.FrameTime();

    auto& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
    re.BeginFrame();

    this->FlushScene();

    FrameBuffer& fb = *re.ScreenFrameBuffer();
    fb.SwapBuffers();
	fb.WaitOnSwapBuffers();

    re.EndFrame();
}

void World::FlushScene()
{
    auto& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
    
    /// 临时修改，清除屏幕
    re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0, 0, 0, 0), 1.0f, 0);
    ///
    
    uint32_t urt;
    auto& app = Context::Instance().AppInstance();
    for (uint32_t pass = 0;; ++ pass)
    {
        urt = app.Update(pass);

        if (urt & App3D::URV_NeedFlush)
        {
            this->Flush(urt);
        }

        if (urt & App3D::URV_Finished)
        {
            break;
        }
    }

    if ((re.Stereo() != STM_None) || (re.DisplayOutput() != DOM_sRGB))
    {
        re.BindFrameBuffer(re.OverlayFrameBuffer());
        re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0, 0, 0, 0), 1.0f, 0);
    }
    this->Flush(App3D::URV_Overlay);
}

}

extern "C"
{
	void MakeRenderWorld(std::unique_ptr<RenderWorker::World>& ptr)
	{
		ptr = CommonWorker::MakeUniquePtr<RenderWorker::World>();
	}
}
