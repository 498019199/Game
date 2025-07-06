#pragma once
#include <base/SceneNode.h>
#include <base/Control.h>

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
    
    void BeginWorld();
    void UpdateScene(float dt);

    void AddSceneObj(const SceneNodePtr& node);
    void RemoveSceneObj(const SceneNodePtr& node);

    void AddRenderable(Renderable* obj);
private:
    SceneNode scene_root_;
    std::vector<std::pair<const RenderTechnique*, std::vector<Renderable*>>> render_queue_;

protected:
    std::vector<SceneNode*> all_scene_nodes_;
    
public:
    ControllerPtr controller_;
    CameraPtr camera_;
};

using WorldPtr = std::shared_ptr<World>;
}
