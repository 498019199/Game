#include <base/Context.h>
#include <common/Util.h>
#include <common/ResIdentifier.h>

#include <filesystem>
#include <fstream>
namespace RenderWorker
{

class Context::Impl final
{
public:
    static Context& Instance()
    {
        if (!context_instance_)
        {
            std::lock_guard<std::mutex> lock(singleton_mutex_);
            if (!context_instance_)
            {
                context_instance_ = MakeUniquePtr<Context>();
            }
        }
        return *context_instance_;
    }

    WinAPP& AppInstance() noexcept;
    RenderEngine& RenderEngineInstance() noexcept;
    RenderFactory& RenderFactoryInstance() noexcept;
    World& WorldInstance() noexcept;
    ResourceLoad& ResourceLoadInstance() noexcept;

private:
	static std::mutex singleton_mutex_;
	static std::unique_ptr<Context> context_instance_;
};
std::mutex Context::Impl::singleton_mutex_;
std::unique_ptr<Context> Context::Impl::context_instance_;

Context& Context::Instance()
{
    return Impl::Instance();
}

WinAPP& Context::AppInstance() noexcept
{
    return pimpl_->AppInstance();
}

RenderEngine& Context::RenderEngineInstance() noexcept
{
    return pimpl_->RenderEngineInstance();
}

RenderFactory& Context::RenderFactoryInstance() noexcept
{
    return pimpl_->RenderFactoryInstance();
}

World& Context::WorldInstance() noexcept
{
    return pimpl_->WorldInstance();
}

ResourceLoad& Context::ResourceLoadInstance() noexcept
{
    return pimpl_->ResourceLoadInstance();
}

}