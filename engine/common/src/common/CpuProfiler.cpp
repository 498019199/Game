#include <common/CpuProfiler.h>

namespace CommonWorker
{

CpuProfiler& CpuProfiler::Instance()
{
	static CpuProfiler inst;
	return inst;
}

void CpuProfiler::BeginFrame()
{
	stack_.clear();
	current_zones_.clear();
	frame_timer_.restart();
}

void CpuProfiler::EndFrame()
{
	while (!stack_.empty())
	{
		PopZone();
	}

	frame_ms_ = static_cast<float>(frame_timer_.elapsed() * 1000.0);
	last_zones_.swap(current_zones_);
	current_zones_.clear();

	float const frame_s = frame_ms_ * 0.001f;
	accumulate_time_ += frame_s;
	++num_frames_;
	if (accumulate_time_ >= 0.5f)
	{
		fps_ = num_frames_ / accumulate_time_;
		accumulate_time_ = 0.f;
		num_frames_ = 0;
	}
}

void CpuProfiler::PushZone(char const* name)
{
	CpuZoneSample sample;
	sample.name = name ? name : "";
	sample.depth = static_cast<int>(stack_.size());
	sample.duration_ms = 0.0;
	current_zones_.push_back(sample);

	ActiveZone zone;
	zone.name = sample.name;
	zone.depth = sample.depth;
	zone.sample_index = current_zones_.size() - 1;
	zone.timer.restart();
	stack_.push_back(std::move(zone));
}

void CpuProfiler::PopZone()
{
	if (stack_.empty())
	{
		return;
	}

	ActiveZone const& top = stack_.back();
	if (top.sample_index < current_zones_.size())
	{
		current_zones_[top.sample_index].duration_ms = top.timer.elapsed() * 1000.0;
	}
	stack_.pop_back();
}

} // namespace CommonWorker
