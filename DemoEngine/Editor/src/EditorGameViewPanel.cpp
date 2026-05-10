
#include <editor/EditorGameViewPanel.h>
#include <editor/EditorManagerD3D11.h>

#include <base/ZEngine.h>

#ifndef EDITOR_DEBUG_MODE
#include <editor/EditorRmlUiHost.h>
#include <RmlUi/Core/Input.h>
#endif

namespace EditorWorker
{
using namespace RenderWorker;

EditorGameViewPanel::EditorGameViewPanel()
{
    
}

EditorGameViewPanel::~EditorGameViewPanel()
{
    
}

void EditorGameViewPanel::OnRender(const EditorSetting& setting)
{
    // 面板大小和位置
    ImGui::SetNextWindowPos(ImVec2((float)setting.hierarchyWidth, (float)setting.mainBarHeight));
    ImGui::SetNextWindowSize(ImVec2((float)setting.gameViewWidth, (float)setting.gameViewHeight));

    // 设置面板具体内容
	if (ImGui::Begin("Game", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse))
	{
        if (ImGui::BeginTabBar("ViewSwitchBar", ImGuiTabBarFlags_None))
        {
            if (ImGui::BeginTabItem("Game"))
            {
                EditorManagerD3D11& editor = checked_cast<EditorManagerD3D11&>(Context::Instance().AppInstance());
#ifndef EDITOR_DEBUG_MODE
                if (EditorRmlUiHost* const rml = editor.RmlUiHost(); rml && rml->Initialized())
                {
                    bool dbg_visible = rml->IsDebuggerVisible();
                    ImGui::Checkbox("RmlUi 调试器", &dbg_visible);
                    rml->SetDebuggerVisible(dbg_visible);
                }
#endif
                ImVec2 const avail = ImGui::GetContentRegionAvail();
                void* const game_srv = editor.GameViewShaderResourceView();
                if (game_srv != nullptr && avail.x > 1.f && avail.y > 1.f)
                {
                    ImGui::Image((ImTextureID)(intptr_t)game_srv, avail);
#ifndef EDITOR_DEBUG_MODE
                    if (EditorRmlUiHost* const rml = editor.RmlUiHost(); rml && rml->Initialized())
                    {
                        bool const hovered = ImGui::IsItemHovered();
                        ImVec2 const min_rect = ImGui::GetItemRectMin();
                        ImVec2 const max_rect = ImGui::GetItemRectMax();
                        ImVec2 const mp = ImGui::GetMousePos();
                        float const iw = max_rect.x - min_rect.x;
                        float const ih = max_rect.y - min_rect.y;
                        int gx = 0;
                        int gy = 0;
                        if (hovered && iw > 1.f && ih > 1.f)
                        {
                            float const nx = (mp.x - min_rect.x) / iw;
                            float const ny = (mp.y - min_rect.y) / ih;
                            gx = static_cast<int>(nx * static_cast<float>(setting.gameViewWidth));
                            gy = static_cast<int>(ny * static_cast<float>(setting.gameViewHeight));
                        }

                        ImGuiIO& io = ImGui::GetIO();
                        int mods = 0;
                        if (io.KeyCtrl)
                        {
                            mods |= static_cast<int>(Rml::Input::KM_CTRL);
                        }
                        if (io.KeyShift)
                        {
                            mods |= static_cast<int>(Rml::Input::KM_SHIFT);
                        }
                        if (io.KeyAlt)
                        {
                            mods |= static_cast<int>(Rml::Input::KM_ALT);
                        }
                        if (io.KeySuper)
                        {
                            mods |= static_cast<int>(Rml::Input::KM_META);
                        }

                        bool const dbg_on = rml->IsDebuggerVisible();
                        if (hovered && dbg_on)
                        {
                            ImGui::SetNextFrameWantCaptureKeyboard(false);
                        }

                        rml->ProcessGameViewPointer(hovered, gx, gy, mods, ImGui::IsMouseClicked(0),
                            ImGui::IsMouseReleased(0), ImGui::IsMouseClicked(1), ImGui::IsMouseReleased(1),
                            ImGui::IsMouseClicked(2), ImGui::IsMouseReleased(2), io.MouseWheelH, io.MouseWheel);

                        rml->ProcessGameViewImGuiKeyboardRelay(hovered && dbg_on, &io);
                    }
#endif
                }
                else
                {
                    ImGui::TextUnformatted("Game view RT unavailable");
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Scene"))
            {
                //EditorDataManager::GetInstance()->isGameView = false;
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }

    ImGui::End();
}

void EditorGameViewPanel::OnResize()
{
    
}

}