#pragma once
//可渲染对象类 头文件
#include <World/SceneNode.h>
#include <Render/RenderEffect.h>
#include <Render/RenderLayout.h>

namespace RenderWorker
{

class Renderable:public SceneNode
{
public:
    struct ConstantBuffer
    {
        float4x4 world;
        float4x4 view;
        float4x4 proj;
    };
public:
    Renderable();
    ~Renderable();
    
    RenderLayout& GetRenderLayout() const;
	RenderLayout& GetRenderLayout(uint32_t lod) const;

	virtual RenderEffect* GetRenderEffect() const;
    virtual RenderTechnique* GetRenderTechnique() const;

    virtual void LodsNum(uint32_t lods);
    virtual uint32_t LodsNum() const;
    virtual void ActiveLod(int32_t lod);
    virtual int32_t ActiveLod() const;

    void Render();
public:
    int32_t active_lod_ = 0;

    // 布局顶点索引
    std::vector<RenderLayoutPtr> rls_;
    // 效果参数合集
    RenderEffectPtr effect_;
    RenderTechnique* technique_ = nullptr;
};
using RenderablePtr = std::shared_ptr<RenderWorker::Renderable>;
}