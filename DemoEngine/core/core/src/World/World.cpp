#include <base/ZEngine.h>
#include <base/App3D.h>
#include <base/Window.h>
#include <world/World.h>
#include <base/Thread.h>

#include <render/RenderEffect.h>
#include <render/Renderable.h>
#include <render/RenderEngine.h>
#include <render/RenderFactory.h>
#include <render/FrameBuffer.h>

namespace RenderWorker
{

World::World()
    :scene_root_(L"SceenRoot", SceneNode::SOA_Cullable),
    overlay_root_(L"OverlayRoot", SceneNode::SOA_Cullable | SceneNode::SOA_Overlay)
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
    COMMON_ASSERT(obj_tech);
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
    std::lock_guard<std::mutex> lock(update_mutex_);

	urt_ = urt;

    scene_root_.Traverse([this](SceneNode& node)
        {
            all_scene_nodes_.push_back(&node);
            return true;
        });
    overlay_root_.Traverse([this](SceneNode& node)
        {
            all_overlay_nodes_.push_back(&node);
            return true;
        });
	auto& scene_nodes = (urt & App3D::URV_Overlay) ? all_overlay_nodes_ : all_scene_nodes_;

    for (auto* node : scene_nodes)
    {
        node->FillVisibleMark(BoundOverlap::No);
    }

    if (!(urt & App3D::URV_Overlay))
    {}
    if (urt & App3D::URV_NeedFlush)
    {}
    if (urt & App3D::URV_Overlay)
    {}

    auto node_visible = MakeUniquePtr<bool[]>(scene_nodes.size());
    for (size_t i = 0; i < scene_nodes.size(); ++i)
    {
        node_visible[i] = true;
    }
    for (size_t i = 0; i < scene_nodes.size(); ++i)
    {
        if (node_visible[i])
        {
            scene_nodes[i]->ForEachComponentOfType<RenderableComponent>(
                [](RenderableComponent& renderable_comp) { renderable_comp.BoundRenderable().ClearInstances(); });
        }
    }

    for (size_t i = 0; i < scene_nodes.size(); ++i)
    {
        if (node_visible[i])
        {
            auto* node = scene_nodes[i];
            node->ForEachComponentOfType<RenderableComponent>([node](RenderableComponent& renderable_comp) {
                auto& renderable = renderable_comp.BoundRenderable();
                if (renderable_comp.Enabled() && (renderable.GetRenderTechnique() != nullptr))
                {
                    if (0 == renderable.NumInstances())
                    {
                        renderable.AddToRenderQueue();
                    }
                    renderable.AddInstance(node);
                }
            });

            ++ num_objects_rendered_;
        }
    }

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

    render_queue_.resize(0);
    
    all_scene_nodes_.clear();
    all_overlay_nodes_.clear();
    urt_ = 0;
}

void World::Update()
{
	auto& context = Context::Instance();
    auto& app = context.AppInstance();
    const float app_time = app.AppTime();
    const float frame_time = app.FrameTime();

    auto& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
    re.BeginFrame();

    if (!update_thread_ && !quit_)
    {
        update_thread_ = context.ThreadPoolInstance().QueueThread([this] { this->UpdateThreadFunc(); });
    }

    {
        std::lock_guard<std::mutex> lock(update_mutex_);

        scene_root_.Traverse([this, app_time, frame_time](SceneNode& node) {
            node.MainThreadUpdate(app_time, frame_time);
            node.UpdateTransforms();

            if (node.Visible())
            {
                node.ForEachComponentOfType<Camera>([this](Camera& camera) {
                    frame_cameras_.push_back(camera.shared_from_this());
                });

                node.ForEachComponentOfType<LightSource>([this](LightSource& light) {
                    frame_lights_.push_back(light.shared_from_this());
                });
            }

            return true;
        });
        scene_root_.UpdatePosBoundSubtree();

        overlay_root_.ClearChildren();
    }

	nodes_updated_ = true;

    this->FlushScene();

    FrameBuffer& fb = *re.ScreenFrameBuffer();
    fb.SwapBuffers();

    frame_cameras_.clear();
    frame_lights_.clear();

	fb.WaitOnSwapBuffers();

    re.EndFrame();

    nodes_updated_ = false;
}

void World::FlushScene()
{
    auto& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
    
    uint32_t urt;
    auto& app = Context::Instance().AppInstance();
    for (uint32_t pass = 0;; ++ pass)
    {
        re.BeginPass();

        urt = app.Update(pass);

        if (urt & App3D::URV_NeedFlush)
        {
            this->Flush(urt);
        }

        re.EndPass();

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

void World::UpdateThreadFunc()
{
    Timer timer;
    float app_time = 0;
    while (!quit_)
    {
        float const frame_time = static_cast<float>(timer.elapsed());
        timer.restart();
        app_time += frame_time;

        if (Context::Instance().AppValid())
        {
            WindowPtr const & win = Context::Instance().AppInstance().MainWnd();
            if (win && win->Active())
            {
                std::lock_guard<std::mutex> lock(update_mutex_);

                auto updater = [app_time, frame_time](SceneNode& node) {
                    node.SubThreadUpdate(app_time, frame_time);
                    return true;
                };
                scene_root_.Traverse(updater);
                overlay_root_.Traverse(updater);
            }

            if (frame_time < update_elapse_)
            {
                CommonWorker::Sleep(static_cast<uint32_t>((update_elapse_ - frame_time) * 1000));
            }
        }
    }
}
}

extern "C"
{
	void MakeRenderWorld(std::unique_ptr<RenderWorker::World>& ptr)
	{
		ptr = CommonWorker::MakeUniquePtr<RenderWorker::World>();
	}
}
