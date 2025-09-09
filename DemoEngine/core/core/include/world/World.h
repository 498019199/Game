#pragma once

#include <world/SceneNode.h>
#include <world/Control.h>

#include <render/Renderable.h>
#include <render/Light.h>
#include <render/RenderEffect.h>

namespace RenderWorker
{

class World
{
public:
    World();
    ~World();

    void Update();

    void AddSceneObj(const SceneNodePtr& node);
    void RemoveSceneObj(const SceneNodePtr& node);
    void AddRenderable(Renderable* obj);

protected:
	void Flush(uint32_t urt);

private:
    void FlushScene();

protected:
    std::vector<SceneNode*> all_scene_nodes_;
    
private:
    SceneNode scene_root_;
    std::vector<std::pair<const RenderTechnique*, std::vector<Renderable*>>> render_queue_;
    
    uint32_t num_renderables_rendered_ {0};

	std::mutex update_mutex_;
};



using WorldPtr = std::shared_ptr<World>;
}
