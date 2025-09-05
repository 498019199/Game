#pragma once
#include <memory>
#include <vector>
#include <render/ElementFormat.h>
#include <render/Viewport.h>

namespace RenderWorker
{
class DepthStencilView;
using DepthStencilViewPtr = std::shared_ptr<DepthStencilView>;
class RenderTargetView;
using RenderTargetViewPtr = std::shared_ptr<RenderTargetView>;
class UnorderedAccessView;
using UnorderedAccessViewPtr = std::shared_ptr<UnorderedAccessView>;

class FrameBuffer
{
public:
    enum class Attachment : uint32_t
    {
        Color0 = 0,
        Color1,
        Color2,
        Color3,
        Color4,
        Color5,
        Color6,
        Color7
    };

    enum ClearBufferMask
    {
        CBM_Color   = 1UL << 0,  // 颜色缓冲区
        CBM_Depth   = 1UL << 1,  // 深度缓冲区
        CBM_Stencil = 1UL << 2   // 模板缓冲区
    };

public:
    FrameBuffer();
    virtual ~FrameBuffer() noexcept;
    
    // 渲染目标的左坐标
    uint32_t Left() const;
    // 渲染目标的顶坐标
    uint32_t Top() const;
    // 渲染目标的宽度
    uint32_t Width() const;
    // 渲染目标的高度
    uint32_t Height() const;

    // 获取视口
    const ViewportPtr& Viewport() const;
    ViewportPtr& Viewport();
    // 设置视口
    void Viewport(const ViewportPtr& viewport);

    void Attach(Attachment att, const RenderTargetViewPtr& view);
    void Detach(Attachment att);
    const RenderTargetViewPtr& AttachedRtv(Attachment att) const;

    void Attach(const DepthStencilViewPtr& view);
    void Detach();
    const DepthStencilViewPtr& AttachedDsv() const;

    void Attach(uint32_t index, const UnorderedAccessViewPtr& view);
    void Detach(uint32_t index);
    const UnorderedAccessViewPtr& AttachedUav(uint32_t index) const;

    virtual void Clear(uint32_t flags, Color const & clr, float depth, int32_t stencil) = 0;
    virtual void Discard(uint32_t flags) = 0;

    virtual void OnBind() = 0;
    virtual void OnUnbind() = 0;

    virtual void SwapBuffers()
    {
    }
    
    virtual void WaitOnSwapBuffers()
    {
    }

    bool Dirty() const
    {
        return views_dirty_;
    }
protected:
    uint32_t	left_ {0};
    uint32_t	top_ {0};
    uint32_t	width_ {0};
    uint32_t	height_ {0};
	
	ViewportPtr viewport_;

    std::vector<RenderTargetViewPtr> rt_views_;
    DepthStencilViewPtr ds_view_;
    std::vector<UnorderedAccessViewPtr> ua_views_;
    
    bool views_dirty_ {false};
};





using FrameBufferPtr = std::shared_ptr<FrameBuffer>;

}