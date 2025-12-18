#pragma once
#include <vector>
#include <base/ZEngine.h>
#include <world/SceneComponent.h>
#include <string_view>
#include <functional>

namespace RenderWorker
{

class SceneNode;
using SceneNodePtr = std::shared_ptr<SceneNode>;

// 场景上对象位置描述
class ZENGINE_CORE_API SceneNode final : public std::enable_shared_from_this<SceneNode>
{
    ZENGINE_NONCOPYABLE(SceneNode);
public:
    enum SOAttrib
    {
        SOA_Cullable = 1UL << 0,
        SOA_Overlay = 1UL << 1,
        SOA_Moveable = 1UL << 2,
        SOA_Invisible = 1UL << 3,
        SOA_NotCastShadow = 1UL << 4,
        SOA_SSS = 1UL << 5
    };

public:
    explicit SceneNode(uint32_t attrib);
    SceneNode(std::wstring_view name, uint32_t attrib);
	~SceneNode();

    std::wstring_view Name() const;
    void Name(std::wstring_view name);

	uint32_t Attrib() const;

    SceneNode* Parent() const;
    void Parent(SceneNode* node);
	std::vector<SceneNode*> const & Children() const;
    void AddChild(const SceneNodePtr& node);
    void RemoveChild(const SceneNodePtr& node);
    void RemoveChild(SceneNode* node);
    
    uint32_t NumComponents() const;
    template <typename T>
    uint32_t NumComponentsOfType() const
    {
        uint32_t ret = 0;
        this->ForEachComponentOfType<T>([&ret]([[maybe_unused]] T& component) {
            ++ret;
        });
        return ret;
    }

    SceneComponent* FirstComponent();
    SceneComponent const* FirstComponent() const;
    SceneComponent* ComponentByIndex(uint32_t i);
    SceneComponent const* ComponentByIndex(uint32_t i) const;
    template <typename T>
    T* FirstComponentOfType()
    {
        for (auto const& component : components_)
        {
            if (auto* casted = NanoRtti::DynCast<T*>(component.get()))
            {
                return casted;
            }
        }
        return nullptr;
    }
    template <typename T>
    T const* FirstComponentOfType() const
    {
        for (auto const& component : components_)
        {
            if (auto const* casted = NanoRtti::DynCast<T*>(component.get()))
            {
                return casted;
            }
        }
        return nullptr;
    }

    void AddComponent(SceneComponentPtr const& component);
    void RemoveComponent(SceneComponentPtr const& component);
    void RemoveComponent(SceneComponent* component);
    void ClearComponents();
    void ReplaceComponent(uint32_t index, SceneComponentPtr const& component);

    void ForEachComponent(std::function<void(SceneComponent&)> const & callback) const;
    template <typename T>
    void ForEachComponentOfType(std::function<void(T&)> const & callback) const
    {
        this->ForEachComponent([&](SceneComponent& component) {
            if (auto* casted = NanoRtti::DynCast<T*>(&component))
            {
                callback(*casted);
            }
        });
    }

    void Traverse(const std::function<bool(SceneNode&)>& callback);
	void UpdatePosBoundSubtree();

    virtual void Update(float dt);

    void TransformToParent(const float4x4& mat);
    void TransformToWorld(const float4x4& mat);

    const float4x4& TransformToParent() const;
    const float4x4& InverseTransformToParent() const;
    const float4x4& TransformToWorld() const;
    const float4x4& InverseTransformToWorld() const;
private:
    std::wstring name_;
    uint32_t attrib_;

    SceneNode* parent_ = nullptr;
	std::vector<SceneNodePtr> children_;

    std::vector<SceneComponentPtr> components_;

    float4x4 xform_to_parent_ {float4x4::Identity()}; 
    float4x4 inv_xform_to_parent_ {float4x4::Identity()};
    mutable float4x4 xform_to_world_ {float4x4::Identity()}; 
    mutable float4x4 inv_xform_to_world_ {float4x4::Identity()}; 

	bool pos_aabb_dirty_ = true;
};







}