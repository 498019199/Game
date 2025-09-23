#pragma once
#include <vector>
#include <base/ZEngine.h>

namespace RenderWorker
{

class SceneNode;
using SceneNodePtr = std::shared_ptr<SceneNode>;

// 场景上对象位置描述
class ZENGINE_CORE_API SceneNode
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
    
    void Traverse(const std::function<bool(SceneNode&)>& callback);

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

    float4x4 xform_to_parent_ {float4x4::Identity()}; 
    float4x4 inv_xform_to_parent_ {float4x4::Identity()};
    mutable float4x4 xform_to_world_ {float4x4::Identity()}; 
    mutable float4x4 inv_xform_to_world_ {float4x4::Identity()}; 
};






}