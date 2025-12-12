#include <editor/EditorInspectorPanel.h>
#include <editor/EditorManagerD3D11.h>
#include <editor/EditorProjectPanel.h>

#include <base/ZEngine.h>
#include <common/Util.h>

#include <render/RenderFactory.h>
#include <audio/AudioFactory.h>

namespace EditorWorker
{

EditorInspectorPanel::EditorInspectorPanel()
{
    
}

EditorInspectorPanel::~EditorInspectorPanel()
{
    
}

void EditorInspectorPanel::OnRender(const EditorSetting& setting)
{
    // 面板大小和位置
    ImGui::SetNextWindowPos(ImVec2((float)setting.hierarchyWidth + (float)setting.gameViewWidth, (float)setting.mainBarHeight));
    ImGui::SetNextWindowSize(ImVec2((float)setting.inspectorWidth, (float)setting.inspectorHeight));
    inspector_width_ = setting.inspectorWidth;

    // 设置面板具体内容
    if (ImGui::Begin("Inspector", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
    {
            auto& d3d_editor = checked_cast<EditorManagerD3D11&>(Context::Instance().AppInstance());
            const auto& pAsset = d3d_editor.GetSelectedAssert();
            if( pAsset )
            {
                switch (d3d_editor.GetAssertType())
                {
                    case AssetType::Script:
                    case AssetType::Text:
                        DrawScript( *(pAsset.get()) );
                        break;

                    case AssetType::Shader:
                    case AssetType::RayTracingShader:
                        DrawShader( *(pAsset.get()) );
                        break;

                    case AssetType::Texture:
                        DrawTexture( *(pAsset.get()) );
                        break;

                    case AssetType::Audio:
                        DrawAudio( *(pAsset.get()) );
                        break;

                    case AssetType::Material:
                    case AssetType::DeferredMaterial:
                    case AssetType::RayTracingMaterial:
                        DrawMaterial( *(pAsset.get()) );
                        break;

                    case AssetType::Model:
                        DrawModel( *(pAsset.get()) );
                        break;
                }
                
            }
    }
    ImGui::End();

}

void EditorInspectorPanel::OnResize()
{
    
}

void EditorInspectorPanel::DrawScript(const AssertBaseInfo& info)
{
    const auto& script_info  = checked_cast<const AssetScriptInfo&>(info);
    std::string title = script_info.name + " (Lua Script)";
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (!ImGui::CollapsingHeader(title.c_str()))
    {    
        return;
    }

    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextUnformatted(script_info.preview.c_str());
    ImGui::PopTextWrapPos();
}

void EditorInspectorPanel::DrawShader(const AssertBaseInfo& info)
{
    const auto& script_info = checked_cast<const AssetScriptInfo&>(info);
    std::string title = script_info.name + " (Shader)";
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (!ImGui::CollapsingHeader(title.c_str()))
    {    
        return;
    }

    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextUnformatted(script_info.preview.c_str());
    ImGui::PopTextWrapPos();
}


void EditorInspectorPanel::DrawTexture(const AssertBaseInfo& info)
{
    const auto& tex_info = checked_cast<const AssetTextureInfo&>(info);
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (!ImGui::CollapsingHeader("Texture"))
        return ;

    ImGui::Text("Name:");
    ImGui::SameLine(120);
    ImGui::Text("%s", tex_info.name.c_str());

    ImGui::Text("Format:");
    ImGui::SameLine(120);
    ImGui::Text("%s", tex_info.format.c_str());

    auto width = tex_info.texture->Width(0);
    auto height = tex_info.texture->Height(0);

    ImGui::Text("Size:");
    ImGui::SameLine(120);
    ImGui::Text("%d x %d", width, height);
    
    uint32_t maxWidth = static_cast<uint32_t>(inspector_width_- 16);
    if (width > maxWidth)
    {
        height = height * maxWidth / width;
        width = maxWidth;
    }

    auto& rf = Context::Instance().RenderFactoryInstance();
    auto srv_ptr = rf.MakeTextureSrv( tex_info.texture );
    auto srv = srv_ptr->GetShaderResourceView();
    ImGui::Image((ImTextureID)(intptr_t)srv, ImVec2((float)width, (float)height));
}

void EditorInspectorPanel::DrawAudio(const AssertBaseInfo& info)
{
    const auto& audio_info = checked_cast<const AssetAudioInfo&>(info);
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (!ImGui::CollapsingHeader("Audio"))
        return;

    ImGui::Text("Name:");
    ImGui::SameLine(120);
    ImGui::Text("%s", audio_info.name.c_str());

    ImGui::Text("Format:");
    ImGui::SameLine(120);
    switch(audio_info.audio_buff_->Format())
    {
        case AudioFormat::AF_Mono8:
            ImGui::Text("%s", "AF_Mono8"); break;
        case AudioFormat::AF_Mono16:
            ImGui::Text("%s", "AF_Mono16"); break;
        case AudioFormat::AF_Stereo8:
            ImGui::Text("%s", "AF_Stereo8"); break;
        case AudioFormat::AF_Stereo16:
            ImGui::Text("%s", "AF_Stereo8"); break;
        default:
            ImGui::Text("%s", "AF_Unknown");
    }
    
    ImGui::Text("Size:");
    ImGui::SameLine(120);
    ImGui::Text("%d", audio_info.audio_buff_->Size() );

    auto& context = Context::Instance();
    AudioFactory& af = context.AudioFactoryInstance();
	AudioEngine& ae = af.AudioEngineInstance();
    ae.AddBuffer(1, af.MakeMusicBuffer(audio_info.audio_buff_, 3));


    ImGui::SetCursorPosX(80);
    if (ImGui::Button("Play", ImVec2(60.0f, 20.0f)))
    {
        ae.Play(1, false);
    }
    
    ImGui::SetCursorPosX(80);
    if (ImGui::Button("Stop", ImVec2(60.0f, 20.0f)))
    {
        ae.Stop(1);
    }
}

void EditorInspectorPanel::DrawModel(const AssertBaseInfo& info)
{
    const auto& model_info = checked_cast<const AssetModelInfo&>(info);

    ImGui::Text("Name:");
    ImGui::SameLine(120);
    ImGui::Text("%s", model_info.name.c_str());
}

void EditorInspectorPanel::DrawMaterial(const AssertBaseInfo& info)
{
    
}

}
