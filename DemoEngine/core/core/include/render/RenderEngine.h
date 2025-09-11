
#pragma once
#include <base/RenderSettings.h>
#include <render/RenderStateObject.h>
#include <render/RenderLayout.h>
#include <render/RenderDeviceCaps.h>
#include <render/FrameBuffer.h>

namespace RenderWorker
{

class RenderEffect;
class RenderTechnique;

class ZENGINE_CORE_API RenderEngine
{
    ZENGINE_NONCOPYABLE(RenderEngine);
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

    virtual void BeginFrame();
    virtual void BeginPass();
    void Render(const RenderEffect& effect, const RenderTechnique& tech, const RenderLayout& rl);
    virtual void EndPass();
    virtual void EndFrame();
    
	void BindFrameBuffer(const FrameBufferPtr& fb);
    // 获取当前渲染目标
    const FrameBufferPtr& CurFrameBuffer() const;
    // 获取默认渲染目标
    const FrameBufferPtr& DefaultFrameBuffer() const;
    // 获取屏幕渲染目标
    const FrameBufferPtr& ScreenFrameBuffer() const;
    const FrameBufferPtr& OverlayFrameBuffer() const;

    StereoMethod Stereo() const;
    void Stereo(StereoMethod method);
    void StereoSeparation(float separation);
    float StereoSeparation() const;

    DisplayOutputMethod DisplayOutput() const;
    void DisplayOutput(DisplayOutputMethod method);
    void PaperWhiteNits(uint32_t nits);
    uint32_t PaperWhiteNits() const;
    void DisplayMaxLuminanceNits(uint32_t nits);
    uint32_t DisplayMaxLuminanceNits() const;
    
    // For debug only, 设置为绘制线框
    void ForceLineMode(bool line);
    bool ForceLineMode() const
    {
        return force_line_mode_;
    }

    virtual void Refresh() const;
    
    // 获取渲染设备能力
    const RenderDeviceCaps& DeviceCaps() const;

protected:

	void Destroy();

private:

	virtual void DoCreateRenderWindow(std::string const & name, RenderSettings const & settings) = 0;
    virtual void DoRender(const RenderEffect& effect, const RenderTechnique& tech, const RenderLayout& rl) = 0;
	virtual void DoBindFrameBuffer(FrameBufferPtr const & fb) = 0;
    virtual void DoBindSOBuffers(const RenderLayoutPtr& rl) = 0;

    virtual void DoDestroy() = 0;

protected:
    FrameBufferPtr cur_frame_buffer_;
    FrameBufferPtr screen_frame_buffer_;
    FrameBufferPtr default_frame_buffers_[4];
    int fb_stage_ {0};
    FrameBufferPtr overlay_frame_buffer_;
    
    // 强制使用线框模式
    bool force_line_mode_ {false}; 
    RenderStateObjectPtr cur_rs_obj_;
    RenderStateObjectPtr cur_line_rs_obj_;
    
    DisplayOutputMethod display_output_method_;
    uint32_t paper_white_;
    uint32_t display_max_luminance_;

    RenderLayoutPtr so_buffers_;

    RenderDeviceCaps caps_;

    StereoMethod stereo_method_ {STM_None};
    float stereo_separation_ {0.0f};
};







}