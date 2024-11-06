#pragma once
#include <core/define.h>
#include <core/IApp.h>
namespace CoreWorker
{
class IRenderDevice
{
public:
    IRenderDevice() = default;
    ~IRenderDevice() = default;
    
    virtual void CreateRenderWindow(const std::string strName, const FWindowDesc& WindowDesc) = 0;
    virtual bool CreateDevice() = 0;
    virtual void Destroy() = 0;

    virtual void Refresh() = 0;
};

using RenderDevicePtr = std::shared_ptr<IRenderDevice>;
}