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
    Context& Instance() const;

    WinAPP& AppInstance() noexcept;
    RenderEngine& RenderEngineInstance() noexcept;
    RenderFactory& RenderFactoryInstance() noexcept;
    World& WorldInstance() noexcept;
    ResourceLoad& ResourceLoadInstance() noexcept;
};

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
    return pimpl_->WorldInstance();
}

}