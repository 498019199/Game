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
}