
#pragma once
#include <Render/RenderStateObject.h>
#include <Render/RenderLayout.h>
#include <Render/RenderDeviceCaps.h>

namespace RenderWorker
{

class RenderEffect;
class RenderTechnique;

class RenderEngine
{
public:
    RenderEngine();
    virtual ~RenderEngine() noexcept;

    // 设置当前渲染状态对象
    void SetStateObject(RenderStateObjectPtr const & rs_obj);
    // 设置当前Stream output目标
    void BindSOBuffers(const RenderLayoutPtr& rl);

    virtual void BeginRender() const = 0;
    virtual void DoRender(const RenderEffect& effect, const RenderTechnique& tech, const RenderLayout& rl) = 0;
    virtual void EndRender() const = 0;

    // For debug only, 设置为绘制线框
    void ForceLineMode(bool line);
    bool ForceLineMode() const
    {
        return force_line_mode_;
    }

    virtual void Refresh();
    
    // 获取渲染设备能力
    const RenderDeviceCaps& DeviceCaps() const;
private:
    virtual void DoBindSOBuffers(const RenderLayoutPtr& rl) = 0;

protected:
    // 强制使用线框模式
    bool force_line_mode_ {false}; 
    RenderStateObjectPtr cur_rs_obj_;
    RenderStateObjectPtr cur_line_rs_obj_;

    RenderLayoutPtr so_buffers_;

    RenderDeviceCaps caps_;
};






}