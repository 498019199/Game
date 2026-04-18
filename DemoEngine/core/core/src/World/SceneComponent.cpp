#include <world/SceneComponent.h>

namespace RenderWorker
{
	SceneComponent::SceneComponent() = default;
	SceneComponent::~SceneComponent() noexcept = default;
    
    void SceneComponent::BindSceneNode(SceneNode* node)
    {
		node_ = node;
    }
    
    SceneNode* SceneComponent::BoundSceneNode() const
    {
        return node_;
    }
    
    bool SceneComponent::Enabled() const
	{
		return enabled_;
	}

	void SceneComponent::Enabled(bool enabled)
	{
		enabled_ = enabled;
	}

	void SceneComponent::SubThreadUpdate(float app_time, float elapsed_time)
	{
		sub_thread_update_event_(*this, app_time, elapsed_time);
	}

	void SceneComponent::MainThreadUpdate(float app_time, float elapsed_time)
	{
		main_thread_update_event_(*this, app_time, elapsed_time);
	}
}