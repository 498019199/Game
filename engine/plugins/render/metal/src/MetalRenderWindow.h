#pragma once

#include "MetalFrameBuffer.h"
#include "MetalUtil.h"

namespace RenderWorker
{

class MetalRenderWindow : public MetalFrameBuffer
{
public:
	MetalRenderWindow(std::string const& name, RenderSettings const& settings);
	~MetalRenderWindow() override;

	void Destroy();
	void SwapBuffers() override;
	void Resize(uint32_t width, uint32_t height) override;

	bool FullScreen() const { return is_full_screen_; }
	void* NativeWindow() const noexcept { return window_; }

private:
	std::string name_;
	void* window_{nullptr};
	bool is_full_screen_{false};
	uint32_t sync_interval_{0};
	ElementFormat depth_stencil_fmt_{EF_Unknown};
	uint32_t sample_count_{1}, sample_quality_{0};
};

using MetalRenderWindowPtr = std::shared_ptr<MetalRenderWindow>;

} // namespace RenderWorker
