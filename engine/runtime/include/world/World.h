#pragma once

#include <world/SceneNode.h>

#include <render/Renderable.h>
#include <render/Light.h>
#include <render/RenderEffect.h>

#include <memory>
#include <optional>
#include <vector>
#include <unordered_map>
#include <future>

namespace RenderWorker
{

class ZENGINE_CORE_API World
{
public:
    World();
    ~World();

    void Update();

    void AddSceneObj(const SceneNodePtr& node);
    void RemoveSceneObj(const SceneNodePtr& node);
    void AddRenderable(Renderable* obj);

    SceneNode& SceneRootNode()
    {
        return scene_root_;
    }
    SceneNode const & SceneRootNode() const
    {
        return scene_root_;
    }

    std::mutex& MutexForUpdate()
    {
        return update_mutex_;
    }
protected:
	void Flush(uint32_t urt);

private:
    void FlushScene();

    void UpdateThreadFunc();

protected:
    std::vector<CameraPtr> frame_cameras_;
    //std::vector<Frustum const*> camera_frustums_;
    std::vector<float4x4> camera_view_projs_;
    std::vector<LightSourcePtr> frame_lights_;

    std::vector<SceneNode*> all_scene_nodes_;
	std::vector<SceneNode*> all_overlay_nodes_;
    
    float update_elapse_;
private:
	uint32_t urt_;

	SceneNode overlay_root_;
    SceneNode scene_root_;
    std::vector<std::pair<const RenderTechnique*, std::vector<Renderable*>>> render_queue_;
    
	uint32_t num_objects_rendered_ {0};
    uint32_t num_renderables_rendered_ {0};

    std::mutex update_mutex_;
    std::optional<std::future<void>> update_thread_;
    volatile bool quit_;

    bool nodes_updated_ = false;
};



using WorldPtr = std::shared_ptr<World>;
}
