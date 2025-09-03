#pragma once
#include <render/FrameBuffer.h>

namespace RenderWorker
{
class D3D11FrameBuffer: public FrameBuffer
{
public:
    D3D11FrameBuffer();

    void OnBind() override;
    void OnUnbind() override;
};
}