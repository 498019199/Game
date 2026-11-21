#pragma once

#include <editor/EditorManager.h>

namespace EditorWorker
{
class EditorManagerMetal final : public EditorManager
{
  public:
    EditorManagerMetal();
    ~EditorManagerMetal() override;

  private:
    void* imgui_text_responder_{};

    void InitializeImGui() override;
    void ShutdownImGui() override;
    void NewImGuiFrame() const override;
    void RenderImGuiDrawData(ImDrawData *draw_data) const override;
    void *GameViewTextureHandle() const override;
    void *TextureHandle(RenderWorker::ShaderResourceView const *srv) const override;
};
}
