#pragma once

#include <World/SceneNode.h>
#include <World/Control.h>

#include <Render/Renderable.h>
#include <Render/Light.h>
#include <Render/RenderEffect.h>

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
