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

SceneNode* SceneNode::Parent() const
{
    return parent_;    
}

const std::vector<SceneNodePtr>& SceneNode::Children() const
{
    return children_;
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

void SceneNode::RemoveChild(const SceneNodePtr& node)
{
    this->RemoveChild(node.get());
}

void SceneNode::RemoveChild(SceneNode* node)
{
    auto iter = std::find_if(children_.begin(), children_.end(), [node](SceneNodePtr const& child) { return child.get() == node; });
    if (iter != children_.end())
    {
        pos_aabb_dirty_ = true;
        node->Parent(nullptr);
        children_.erase(iter);

        this->EmitSceneChanged();
    }
}

void SceneNode::ClearChildren()
{
    for (auto const& child : children_)
    {
        child->Parent(nullptr);
    }

    pos_aabb_dirty_ = true;
    children_.clear();

    this->EmitSceneChanged();
}

void SceneNode::AddComponent(SceneComponentPtr const& component)
{
    COMMON_ASSERT(component);

    auto* curr_node = component->BoundSceneNode();
    if (curr_node != nullptr)
    {
        curr_node->RemoveComponent(component);
    }

    components_.push_back(component);
    component->BindSceneNode(this);
    pos_aabb_dirty_ = true;
}

void SceneNode::RemoveComponent(SceneComponentPtr const& component)
{
    this->RemoveComponent(component.get());
}

void SceneNode::RemoveComponent(SceneComponent* component)
{
    auto iter =
        std::find_if(components_.begin(), components_.end(), [component](SceneComponentPtr const& comp) { return comp.get() == component; });
    if (iter != components_.end())
    {
        components_.erase(iter);
        component->BindSceneNode(nullptr);
        pos_aabb_dirty_ = true;
    }
}

void SceneNode::ClearComponents()
{
    components_.clear();
    pos_aabb_dirty_ = true;
}

void SceneNode::ReplaceComponent(uint32_t index, SceneComponentPtr const& component)
{
    COMMON_ASSERT(component);

    auto* curr_node = component->BoundSceneNode();
    if (curr_node != this)
    {
        if (curr_node != nullptr)
        {
            curr_node->RemoveComponent(component);
        }

        if (components_[index] != nullptr)
        {
            components_[index]->BindSceneNode(nullptr);
        }

        component->BindSceneNode(this);
        components_[index] = component;
        pos_aabb_dirty_ = true;
    }
}

void SceneNode::ForEachComponent(std::function<void(SceneComponent&)> const& callback) const
{
    for (auto const& component : components_)
    {
        if (component)
        {
            callback(*component);
        }
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

uint32_t SceneNode::NumComponents() const
{
    return static_cast<uint32_t>(components_.size());
}

SceneComponent* SceneNode::FirstComponent()
{
    return this->ComponentByIndex(0);
}

SceneComponent const* SceneNode::FirstComponent() const
{
    return this->ComponentByIndex(0);
}

SceneComponent* SceneNode::ComponentByIndex(uint32_t i)
{
    return components_[i].get();
}

SceneComponent const* SceneNode::ComponentByIndex(uint32_t i) const
{
    return components_[i].get();
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

void SceneNode::FillVisibleMark(BoundOverlap vm)
{
    visible_marks_.fill(vm);
}

void SceneNode::VisibleMark(uint32_t camera_index, BoundOverlap vm)
{
    COMMON_ASSERT(camera_index < visible_marks_.size());
    visible_marks_[camera_index] = vm;
}

BoundOverlap SceneNode::VisibleMark(uint32_t camera_index) const
{
    COMMON_ASSERT(camera_index < visible_marks_.size());
    return visible_marks_[camera_index];
}

uint32_t SceneNode::Attrib() const
{
    return attrib_;
}

bool SceneNode::Visible() const
{
    return (0 == (attrib_ & SOA_Invisible));
}

void SceneNode::Visible(bool vis)
{
    if (vis)
    {
        attrib_ &= ~SOA_Invisible;
    }
    else
    {
        attrib_ |= SOA_Invisible;
    }

    for (auto const & child : children_)
    {
        child->Visible(vis);
    }
}

void SceneNode::Parent(SceneNode* so)
{
    parent_ = so;

    pos_aabb_dirty_ = true;
    //updated_ = false;
}

void SceneNode::EmitSceneChanged()
{
    // auto& context = Context::Instance();
    // if (context.SceneManagerValid())
    // {
    //     auto* node = this;
    //     while (node->Parent() != nullptr)
    //     {
    //         node = node->Parent();
    //     }

    //     auto& scene_mgr = context.SceneManagerInstance();
    //     if (node == &scene_mgr.SceneRootNode())
    //     {
    //         scene_mgr.OnSceneChanged();
    //     }
    // }
}

void SceneNode::Update(float dt)
{
    
}

}