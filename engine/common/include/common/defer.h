#pragma once

#include <functional>
namespace CommonWorker
{
class Defer 
{
public:
	explicit Defer(std::function<void()> && func) 
    {
        _func = std::move(func);
	}

    Defer(const Defer&) = delete;
    Defer(Defer&&) = default;
    Defer& operator = (const Defer&) = delete;
    Defer& operator = (Defer&&) = delete;

	~Defer() 
    {
		_func();
	}

private:
	std::function<void()> _func;
};
}

#define defer_name(name, count) name##count##_common_private
#define defer_link(class, count) defer_name(class, count)
#define defer(expr) auto defer_link(Defer, __COUNTER__) = CommonWorker::Defer(expr)