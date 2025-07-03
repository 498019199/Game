#pragma once

#include <mutex>
#include <atomic>

namespace RenderWorker
{
template <typename ClassType>
class Singleton 
{
    Singleton() = delete;
    Singleton(const Singleton &) = delete;
    Singleton(Singleton &&) = delete;
    Singleton<ClassType> &operator=(const Singleton &) = delete;

private:
    static std::mutex _lock;
    static std::atomic<ClassType *> _instance;

public:
    template <typename... Args>
    static ClassType *Instance(Args &&... vArgs) 
    {
        auto current = _instance.load(std::memory_order_acquire);
        if (!current) 
        {
            std::lock_guard<std::mutex> locker(_lock);
            current = _instance.load(std::memory_order_relaxed);
            if (!current) 
            {
                current = new ClassType(std::forward<Args>(vArgs)...);
                _instance.store(current, std::memory_order_release);
            }
        }
        return current;
    }

    static void Destroy() 
    {
        auto current = _instance.load(std::memory_order_acquire);
        if (current) 
        {
            std::lock_guard<std::mutex> locker(_lock);
            current = _instance.load(std::memory_order_relaxed);
            if (current) 
            {
                delete current;
                _instance.store(nullptr, std::memory_order_release);
            }
        }
    }
};

template <typename T>
std::mutex Singleton<T>::_lock;

template <typename T>
std::atomic<T *> Singleton<T>::_instance;
}