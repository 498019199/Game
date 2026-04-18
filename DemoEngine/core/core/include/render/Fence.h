#pragma once
#include <base/ZEngine.h>

// CPU 与 GPU 之间同步操作的核心机制
namespace RenderWorker
{
class ZENGINE_CORE_API Fence
{
    ZENGINE_NONCOPYABLE(Fence);
public:
    enum FenceType
    {
        FT_Render,      // 渲染
        FT_Compute,     // 计算
        FT_Copy         // 拷贝
    };

public:
    Fence() = default;
    ~Fence() = default;

    virtual uint64_t Signal(FenceType ft) = 0;
    virtual void Wait(uint64_t id) = 0;
    virtual bool Completed(uint64_t id) = 0;
};
using FencePtr = std::shared_ptr<Fence>;
}