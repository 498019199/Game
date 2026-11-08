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

    bool Enabled() const;
	void Enabled(bool enabled);

    using UpdateEvent = Signal::Signal<void(SceneComponent&, float, float)>;
    UpdateEvent& OnSubThreadUpdate()
    {
        return sub_thread_update_event_;
    }
    UpdateEvent& OnMainThreadUpdate()
    {
        return main_thread_update_event_;
    }
    virtual void SubThreadUpdate(float app_time, float elapsed_time);
	virtual void MainThreadUpdate(float app_time, float elapsed_time);
protected:
    SceneNode* node_ {nullptr};
	bool enabled_ {true};

    UpdateEvent sub_thread_update_event_;
    UpdateEvent main_thread_update_event_;
};

}