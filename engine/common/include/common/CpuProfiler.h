#pragma once

#include <common/Timer.h>

#include <cstdint>
#include <vector>

namespace CommonWorker
{

struct CpuZoneSample
{
	char const* name = "";
	double duration_ms = 0.0;
	int depth = 0;
};

/// Process-local frame/zone timings for the Editor ImGui profiler panel.
/// Independent of Tracy (can run with or without ZENGINE_ENABLE_TRACY).
class CpuProfiler
{
public:
	static CpuProfiler& Instance();

	void BeginFrame();
	void EndFrame();

	void PushZone(char const* name);
	void PopZone();

	float Fps() const noexcept { return fps_; }
	float FrameMs() const noexcept { return frame_ms_; }

	std::vector<CpuZoneSample> const& LastFrameZones() const noexcept { return last_zones_; }

private:
	CpuProfiler() = default;

	struct ActiveZone
	{
		char const* name = "";
		int depth = 0;
		size_t sample_index = 0;
		Timer timer;
	};

	std::vector<ActiveZone> stack_;
	std::vector<CpuZoneSample> current_zones_;
	std::vector<CpuZoneSample> last_zones_;

	Timer frame_timer_;
	float frame_ms_ = 0.f;
	float fps_ = 0.f;
	float accumulate_time_ = 0.f;
	uint32_t num_frames_ = 0;
};

class ScopedCpuZone
{
public:
	explicit ScopedCpuZone(char const* name)
	{
		CpuProfiler::Instance().PushZone(name);
	}

	~ScopedCpuZone()
	{
		CpuProfiler::Instance().PopZone();
	}

	ScopedCpuZone(ScopedCpuZone const&) = delete;
	ScopedCpuZone& operator=(ScopedCpuZone const&) = delete;
};

} // namespace CommonWorker
