#pragma once

#include "D3D12FrameBuffer.h"
#include "D3D12Util.h"

namespace RenderWorker
{

class D3D12RenderWindow : public D3D12FrameBuffer
{
public:
	D3D12RenderWindow(std::string const& name, RenderSettings const& settings);
	~D3D12RenderWindow() override;

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

using D3D12RenderWindowPtr = std::shared_ptr<D3D12RenderWindow>;

} // namespace RenderWorker
