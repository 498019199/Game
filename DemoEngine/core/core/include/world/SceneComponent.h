#pragma once
#include <base/ZEngine.h>
#include <world/SceneNode.h>
#include <NanoRtti/NanoRtti.hpp>

namespace RenderWorker
{

class ZENGINE_CORE_API SceneComponent
{
    ZENGINE_NONCOPYABLE(SceneComponent);
public:
    NANO_RTTI_REGISTER_RUNTIME_CLASS()
    
    SceneComponent();
    virtual ~SceneComponent() noexcept;

    virtual void BindSceneNode(SceneNode* node);
    SceneNode* BoundSceneNode() const;
protected:
    SceneNode* node_ = nullptr;
};

}