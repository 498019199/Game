
#pragma once
#include <base/RenderSettings.h>
#include <render/RenderStateObject.h>
#include <render/RenderLayout.h>
#include <render/RenderDeviceCaps.h>

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

#if ZENGINE_IS_DEV_PLATFORM
    virtual void* GetD3DDevice();
    virtual void* GetD3DDeviceImmContext();
#endif //ZENGINE_IS_DEV_PLATFORM

    // 建立渲染窗口
	void CreateRenderWindow(std::string const & name, RenderSettings& settings);
	void DestroyRenderWindow();

    virtual void BeginRender() const = 0;
    virtual void DoRender(const RenderEffect& effect, const RenderTechnique& tech, const RenderLayout& rl) = 0;
    virtual void EndRender() const = 0;

    // For debug only, 设置为绘制线框
    void ForceLineMode(bool line);
    bool ForceLineMode() const
    {
        return force_line_mode_;
    }

    virtual void Refresh() const;
    
    // 获取渲染设备能力
    const RenderDeviceCaps& DeviceCaps() const;
private:

	virtual void DoCreateRenderWindow(std::string const & name, RenderSettings const & settings) = 0;
    virtual void DoBindSOBuffers(const RenderLayoutPtr& rl) = 0;

protected:
    // 强制使用线框模式
    bool force_line_mode_ {false}; 
    RenderStateObjectPtr cur_rs_obj_;
    RenderStateObjectPtr cur_line_rs_obj_;

    RenderLayoutPtr so_buffers_;

    RenderDeviceCaps caps_;

    StereoMethod stereo_method_;
    float stereo_separation_;
};




}