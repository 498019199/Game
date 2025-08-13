#include <base/Context.h>
#include <base/WinApp.h>

#include <world/World.h>

#include <render/RenderEffect.h>
#include <render/Renderable.h>
#include <render/RenderEngine.h>

namespace RenderWorker
{

World::World()
{
}

World::~World()
{
    if(controller_)
    {
        controller_->DetachCamera();
    }
}

void World::BeginWorld()
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

void World::UpdateScene(float dt)
{
    auto& re = Context::Instance().RenderEngineInstance();
    re.BeginRender();

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
    }
    re.EndRender();

    for (auto& items : render_queue_)
    {
        for (auto const & item : items.second)
        {
            item->Update(dt);
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
