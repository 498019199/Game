#pragma once
#include <memory>

namespace EditorWorker
{
class EditorPanel
{
public:
    EditorPanel() = default;
    ~EditorPanel() = default;

    virtual void OnRender() = 0;
    virtual void OnResize() = 0;
private:
};
using EditorPanelPtr = std::shared_ptr<EditorPanel>;
}