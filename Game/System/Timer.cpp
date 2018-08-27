#include "Timer.h"
#include <chrono>
Timer::Timer()
{
	this->ReStart();
}

void Timer::ReStart()
{
	m_fTime = this->CurrentTime();
}

double Timer::CurrentTime()
{
	std::chrono::high_resolution_clock::time_point tp = std::chrono::high_resolution_clock::now();
	return std::chrono::duration_cast<std::chrono::duration<double>>(tp.time_since_epoch()).count();
}

double Timer::Elapsed()
{
	return this->CurrentTime() - m_fTime;
}

double Timer::ElapseMax()
{
	return std::chrono::duration<double>::max().count();
}

double Timer::ElapseMin()
{
	return std::chrono::duration<double>::min().count();
}
