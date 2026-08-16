#pragma once

#include <editor/EditorManager.h>

namespace EditorWorker
{
class EditorManagerSDL3 final : public EditorManager
{
public:
    EditorManagerSDL3();
    ~EditorManagerSDL3() override;

#ifdef ZENGINE_PLATFORM_WINDOWS_DESKTOP
    void ProcessWindowMessage(HWND, UINT, WPARAM, LPARAM) override {}
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
