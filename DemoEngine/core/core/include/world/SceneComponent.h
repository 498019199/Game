#pragma once
#include <base/ZEngine.h>
#include <world/SceneNode.h>

namespace RenderWorker
{

class ZENGINE_CORE_API SceneComponent
{
    ZENGINE_NONCOPYABLE(SceneComponent);
public:
    SceneComponent();
    virtual ~SceneComponent() noexcept;

    virtual void BindSceneNode(SceneNode* node);
    SceneNode* BoundSceneNode() const;
protected:
    SceneNode* node_ = nullptr;
};

}