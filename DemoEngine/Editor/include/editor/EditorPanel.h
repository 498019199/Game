#pragma once

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
}