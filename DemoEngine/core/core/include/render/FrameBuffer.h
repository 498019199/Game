#pragma once


namespace RenderWorker
{
class FrameBuffer
{
public:
    FrameBuffer() = default;
    ~FrameBuffer() = default;
    
    virtual void OnBind() = 0;
    virtual void OnUnbind() = 0;

    virtual void SwapBuffers()
    {
    }
    virtual void WaitOnSwapBuffers()
    {
    }
};
}