#pragma once

#include <editor/EditorManager.h>

namespace EditorWorker
{
class EditorManagerD3D11 final : public EditorManager
{
public:
    EditorManagerD3D11();
    ~EditorManagerD3D11() override;

#ifdef ZENGINE_PLATFORM_WINDOWS_DESKTOP
    void ProcessWindowMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;
#endif

private:
    void InitializeImGui() override;
    void ShutdownImGui() override;
    void NewImGuiFrame() const override;
    void RenderImGuiDrawData(ImDrawData* draw_data) const override;
    void* GameViewTextureHandle() const override;
    void* TextureHandle(RenderWorker::ShaderResourceView const* srv) const override;
};
}
