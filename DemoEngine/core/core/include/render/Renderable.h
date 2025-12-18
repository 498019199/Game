#pragma once
//可渲染对象类 头文件
#include <render/RenderEffect.h>
#include <render/RenderLayout.h>
#include <render/RenderMaterial.h>

namespace RenderWorker
{

class ZENGINE_CORE_API Renderable
{
    ZENGINE_NONCOPYABLE(Renderable);
public:
    struct ConstantBuffer
    {
        float4x4 world;
        float4x4 view;
        float4x4 proj;
    };
public:
    Renderable();
	explicit Renderable(std::wstring_view name);
    virtual ~Renderable();
    
    virtual RenderLayout& GetRenderLayout() const;
	virtual RenderLayout& GetRenderLayout(uint32_t lod) const;
	virtual const std::wstring& Name() const;

    virtual RenderEffectPtr const & GetRenderEffect() const
    {
        return effect_;
    }
    virtual RenderTechnique* GetRenderTechnique() const
    {
        return technique_;
    }

    virtual const AABBox& PosBound() const;
	virtual const AABBox& TexcoordBound() const;

    virtual void NumLods(uint32_t lods);
    virtual uint32_t NumLods() const;
    virtual void ActiveLod(int32_t lod);
    virtual int32_t ActiveLod() const;

    void Render();

    virtual bool HWResourceReady() const
    {
        return true;
    }

    virtual void Material(const RenderMaterialPtr& mtl);
    virtual const RenderMaterialPtr&  Material() const
    {
        return mtl_;
    }

    virtual void IsSkinned(bool is_skinned)
    {
        is_skinned_ = is_skinned;
    }
protected:
    std::wstring name_;
    AABBox pos_aabb_;
	AABBox tc_aabb_;

    int32_t active_lod_ = 0;

    // 布局顶点索引
    std::vector<RenderLayoutPtr> rls_;
    // 效果参数合集
    RenderEffectPtr effect_;
    RenderTechnique* technique_ {nullptr};

	RenderMaterialPtr mtl_;

    bool is_skinned_ {false};
};
using RenderablePtr = std::shared_ptr<RenderWorker::Renderable>;


// TODO: Consider merging this with Renderable
class ZENGINE_CORE_API RenderableComponent : public SceneComponent
{
public:
    NANO_RTTI_REGISTER_RUNTIME_CLASS(SceneComponent)

    explicit RenderableComponent(RenderablePtr const& renderable);

    SceneComponentPtr Clone() const override;

    Renderable& BoundRenderable() const;

    template <typename T>
    T& BoundRenderableOfType() const
    {
        return checked_cast<T&>(this->BoundRenderable());
    }

private:
    RenderablePtr renderable_;
};
using RenderableComponentPtr = std::shared_ptr<RenderWorker::RenderableComponent>;
}