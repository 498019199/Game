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
    
}