#pragma once
#include <vector>
#include <common/common.h>

namespace RenderWorker
{

class SceneNode;
using SceneNodePtr = std::shared_ptr<SceneNode>;

// 场景上对象位置描述
class SceneNode
{
public:
    SceneNode* Parent() const;
    void Parent(SceneNode* node);
	std::vector<SceneNode*> const & Children() const;
    void AddChild(const SceneNodePtr& node);
    void RemoveChild(const SceneNodePtr& node);

    void Traverse(const std::function<bool(SceneNode&)>& callback);

    virtual void Update(float dt);

    void TransformToParent(const float4x4& mat);
    void TransformToWorld(const float4x4& mat);

    const float4x4& TransformToParent() const;
    const float4x4& InverseTransformToParent() const;
    const float4x4& TransformToWorld() const;
    const float4x4& InverseTransformToWorld() const;
private:
    SceneNode* parent_ = nullptr;
	std::vector<SceneNodePtr> children_;

    float4x4 xform_to_parent_ {float4x4::Identity()}; 
    float4x4 inv_xform_to_parent_ {float4x4::Identity()};
    mutable float4x4 xform_to_world_ {float4x4::Identity()}; 
    mutable float4x4 inv_xform_to_world_ {float4x4::Identity()}; 
};




}