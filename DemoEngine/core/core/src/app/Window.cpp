#include <base/Window.h>

namespace RenderWorker
{
	void Window::UpdateDpiScale(float scale)
	{
		dpi_scale_ = scale;

		float const max_dpi_scale = Context::Instance().Config().graphics_cfg.max_dpi_scale;
		if (max_dpi_scale > 0)
		{
			effective_dpi_scale_ = std::min(max_dpi_scale, dpi_scale_);
		}
		else
		{
			effective_dpi_scale_ = dpi_scale_;
		}
	}
}