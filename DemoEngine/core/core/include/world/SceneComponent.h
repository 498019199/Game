#pragma once
#include <base/ZEngine.h>
#include <NanoRtti/NanoRtti.hpp>

namespace RenderWorker
{
class SceneNode;
class SceneComponent;
using SceneComponentPtr = std::shared_ptr<SceneComponent>;

class ZENGINE_CORE_API SceneComponent
{
    ZENGINE_NONCOPYABLE(SceneComponent);
public:
    NANO_RTTI_REGISTER_RUNTIME_CLASS()
    
    SceneComponent();
    virtual ~SceneComponent() noexcept;

    virtual SceneComponentPtr Clone() const = 0;

    template <typename T>
    bool IsOfType() const
    {
        return (NanoRtti::DynCast<T const*>(this) != nullptr);
    }
    
    virtual void BindSceneNode(SceneNode* node);
    SceneNode* BoundSceneNode() const;
protected:
    SceneNode* node_ = nullptr;
};

}