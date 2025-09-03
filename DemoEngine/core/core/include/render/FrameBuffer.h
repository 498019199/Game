#pragma once
#include <memory>
#include <vector>

namespace RenderWorker
{
class FrameBuffer
{
public:
    FrameBuffer();
    virtual ~FrameBuffer() noexcept;
    
    virtual void OnBind() = 0;
    virtual void OnUnbind() = 0;

    virtual void SwapBuffers()
    {
    }
    
    virtual void WaitOnSwapBuffers()
    {
    }

    // 渲染目标的左坐标
    uint32_t Left() const;
    // 渲染目标的顶坐标
    uint32_t Top() const;
    // 渲染目标的宽度
    uint32_t Width() const;
    // 渲染目标的高度
    uint32_t Height() const;
protected:
    uint32_t	left_ {0};
    uint32_t	top_ {0};
    uint32_t	width_ {0};
    uint32_t	height_ {0};
	
    bool views_dirty_ {false};
};


}