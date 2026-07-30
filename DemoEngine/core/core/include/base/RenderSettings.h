#pragma once

#include <vector>
#include <render/ElementFormat.h>

namespace RenderWorker
{
// 建立渲染窗口的设置
/////////////////////////////////////////////////////////////////////////////////
struct RenderSettings
{
	RenderSettings()
		: hide_win(false), full_screen(false),
			left(0), top(0),
			color_fmt(EF_ARGB8), depth_stencil_fmt(EF_D16),
			sample_count(1), sample_quality(0),
			hdr(false), fft_lens_effects(false), ppaa(false), gamma(false), color_grading(false),
			bloom(0.25f), blue_shift(true), keep_screen_on(true),
			stereo_method(STM_None), stereo_separation(0),
			display_output_method(DOM_sRGB), paper_white(100), display_max_luminance(100),
			max_dpi_scale(2)
	{
	}

	bool	hide_win;
	bool	full_screen;
	int		left;
	int		top;
	int		width;
	int		height;
	ElementFormat color_fmt;// 后台缓冲区像素格式
	ElementFormat depth_stencil_fmt;
	uint32_t sample_count;
	uint32_t sample_quality;
	uint32_t sync_interval;
	bool hdr;
	bool fft_lens_effects;
	bool ppaa;
	bool gamma;
	bool color_grading;

	float bloom;
	bool blue_shift;

	bool keep_screen_on;

	StereoMethod stereo_method;
	float stereo_separation;

	DisplayOutputMethod display_output_method;
	uint32_t paper_white;
	uint32_t display_max_luminance;

	float max_dpi_scale;

	std::vector<std::pair<std::string, std::string>> options;

	bool debug_context = false;
};
}
