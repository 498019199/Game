#pragma once
#include <base/App3D.h>
#include <editor/EditorPanel.h>
#include <editor/EditorProjectPanel.h>

#include <vector>

namespace EditorWorker
{
class EditorManagerD3D11: public RenderWorker::App3D
{
public:
    EditorManagerD3D11();
    ~EditorManagerD3D11();

    virtual void OnCreate() override;
    virtual void OnDestroy() override;

    void SetWindowSize(int hWidth, int pHeight, int iWidth);

private :
    virtual uint32_t DoUpdate(uint32_t pass) override;

private:
    std::vector<EditorPanelPtr> panel_list_;
    EditorSetting setting_;
};










}