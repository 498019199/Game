#include <world/SceneNode.h>

namespace RenderWorker
{

SceneNode* SceneNode::Parent() const
{
    return parent_;    
}

void SceneNode::Parent(SceneNode* node)
{
    parent_ = node;
}

void SceneNode::AddChild(const SceneNodePtr& node)
{
    auto iter = std::find(children_.begin(), children_.end(), node);
    if (iter == children_.end())
    {
        node->Parent(this);
        children_.push_back(node);
    }
}

void SceneNode::Traverse(const std::function<bool(SceneNode&)>& callback)
{
    if (callback(*this))
    {
        for (auto const& child : children_)
        {
            child->Traverse(callback);
        }
    }
}

void SceneNode::RemoveChild(const SceneNodePtr& node)
{
    auto iter = std::find_if(children_.begin(), children_.end(), [node](const SceneNodePtr& child) { return child == node; });
    if (iter != children_.end())
    {
        node->Parent(nullptr);
        children_.erase(iter);
    }
}


void SceneNode::TransformToParent(const float4x4& mat)
{
    xform_to_parent_ = mat;
    inv_xform_to_parent_ = MathWorker::Inverse(mat);
}

void SceneNode::TransformToWorld(const float4x4& mat)
{
    if (parent_)
    {
        xform_to_parent_ = mat * parent_->InverseTransformToWorld();
    }
    else
    {
        xform_to_parent_ = mat;
    }
    inv_xform_to_parent_ = MathWorker::Inverse(mat);
}

const float4x4& SceneNode::TransformToParent() const
{
    return xform_to_parent_;
}

const float4x4& SceneNode::InverseTransformToParent() const
{
    return inv_xform_to_parent_;
}

const float4x4& SceneNode::TransformToWorld() const
{
    if(nullptr == parent_)
    {
        return xform_to_parent_;
    }
    else
    {
        auto* parent = Parent();
        xform_to_world_ = xform_to_parent_;
        while (nullptr != parent)
        {
            xform_to_world_ *= parent->TransformToParent();
            parent = parent->Parent();
        }
        return xform_to_world_;
    }
}

const float4x4& SceneNode::InverseTransformToWorld() const
{
    if (parent_ == nullptr)
    {
        return inv_xform_to_parent_;
    }
    else
    {
        inv_xform_to_world_ = MathWorker::Inverse(TransformToWorld());
        return inv_xform_to_world_;
    }
}

void SceneNode::Update(float dt)
{
    
}

}