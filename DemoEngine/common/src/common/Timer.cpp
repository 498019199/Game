#include <common/Timer.h>

#include <limits>
#include <chrono>

namespace RenderWorker
{

Timer::Timer()
{
    restart();
}

double Timer::elapsed() const
{
    return this->current_time() - start_time_;
}

double Timer::elapsed_max() const
{
    return std::chrono::duration<double>::max().count();
}

double Timer::elapsed_min() const
{
    return std::chrono::duration<double>::min().count();
}

void Timer::restart()
{
    start_time_ = this->current_time();
}

double Timer::current_time() const
{
	std::chrono::high_resolution_clock::time_point const tp = std::chrono::high_resolution_clock::now();
	return std::chrono::duration_cast<std::chrono::duration<double>>(tp.time_since_epoch()).count();
}
}