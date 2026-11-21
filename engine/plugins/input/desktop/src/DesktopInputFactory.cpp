#include <base/ZEngine.h>
#include <base/InputFactory.h>

#include "DesktopInput.hpp"

namespace RenderWorker
{
class DesktopInputFactory : public InputFactory
{
  public:
    std::wstring const &Name() const override
    {
        static std::wstring const name(L"Desktop Input Factory");
        return name;
    }

  private:
    std::unique_ptr<InputEngine> DoMakeInputEngine() override
    {
        return MakeUniquePtr<DesktopInputEngine>();
    }

    void DoSuspend() override
    {
    }
    void DoResume() override
    {
    }
};
}

extern "C"
{
    ZENGINE_SYMBOL_EXPORT void MakeInputFactory(std::unique_ptr<RenderWorker::InputFactory> &ptr)
    {
        ptr = RenderWorker::MakeUniquePtr<RenderWorker::DesktopInputFactory>();
    }
}
