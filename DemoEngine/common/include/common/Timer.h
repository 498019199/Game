#pragma once

namespace RenderWorker
{

class Timer
{
public:
    Timer();
    ~Timer() = default;

    // return elapsed time in seconds
	double elapsed() const;
    // return estimated maximum value for elapsed()
	double elapsed_max() const;
    // return minimum value for elapsed()
	double elapsed_min() const;

    void restart();
    double current_time() const;
private:
    double start_time_ = 0.f;
};

}






