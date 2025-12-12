#include <world/SceneNode.h>

namespace RenderWorker
{
SceneNode::SceneNode(uint32_t attrib)
    : attrib_(attrib)
{
}

SceneNode::SceneNode(std::wstring_view name, uint32_t attrib)
: SceneNode(attrib)
{
    name_ = std::wstring(name);
}

SceneNode::~SceneNode()
{
    if (parent_)
    {
        parent_->RemoveChild(this);
    }
}

std::wstring_view SceneNode::Name() const
{
    return name_;
}

void SceneNode::Name(std::wstring_view name)
{
    name_ = std::wstring(name);
}

uint32_t SceneNode::Attrib() const
{
    return attrib_;
}

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

void SceneNode::UpdatePosBoundSubtree()
{
    
}

void SceneNode::RemoveChild(const SceneNodePtr& node)
{
    this->RemoveChild(node.get());
}

void SceneNode::RemoveChild(SceneNode* node)
{
    auto iter = std::find_if(children_.begin(), children_.end(), [node](const SceneNodePtr& child) { return child.get() == node; });
    if (iter != children_.end())
    {
        node->Parent(nullptr);
        children_.erase(iter);
    }
}

void SceneNode::TransformToParent(const float4x4& mat)
{
    xform_to_parent_ = mat;
    inv_xform_to_parent_ = MathWorker::inverse(mat);
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
    inv_xform_to_parent_ = MathWorker::inverse(mat);
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
        inv_xform_to_world_ = MathWorker::inverse(TransformToWorld());
        return inv_xform_to_world_;
    }
}

void SceneNode::Update(float dt)
{
    
}

}