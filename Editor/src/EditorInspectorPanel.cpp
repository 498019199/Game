#include <editor/EditorInspectorPanel.h>
#include <editor/EditorManagerD3D11.h>
#include <editor/EditorProjectPanel.h>

#include <base/ZEngine.h>
#include <common/Util.h>

#include <render/RenderFactory.h>
#include <base/AudioFactory.h>

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

void EditorInspectorPanel::DrawScript(AssertBaseInfo& info)
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

void EditorInspectorPanel::DrawShader(AssertBaseInfo& info)
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


void EditorInspectorPanel::DrawTexture(AssertBaseInfo& info)
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

void EditorInspectorPanel::DrawAudio(AssertBaseInfo& info)
{
    auto& audio_info = checked_cast<AssetAudioInfo&>(info);
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

void EditorInspectorPanel::DrawModel(AssertBaseInfo& info)
{
    // auto& model_info = checked_cast<AssetModelInfo&>(info);

    // ImGui::Text("Name:");
    // ImGui::SameLine(120);
    // ImGui::Text("%s", model_info.name.c_str());

    // if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_DefaultOpen))
    // {
    //     static RenderWorker::RenderModel* editing_model = nullptr;
    //     static float pos[3] = { 0.0f, 0.0f, 0.0f };
    //     static float rot[3] = { 0.0f, 0.0f, 0.0f };
    //     static float scl[3] = { 1.0f, 1.0f, 1.0f };

    //     auto node = model_info.model->RootNode();
    //     if (editing_model != model_info.model.get())
    //     {
    //         editing_model = model_info.model.get();

    //         RenderWorker::float3 scale;
    //         RenderWorker::quater rotation;
    //         RenderWorker::float3 translation;
    //         MathWorker::decompose(scale, rotation, translation, node->TransformToParent());

    //         pos[0] = translation.x();
    //         pos[1] = translation.y();
    //         pos[2] = translation.z();
    //         rot[0] = rot[1] = rot[2] = 0.0f;
    //         scl[0] = scale.x();
    //         scl[1] = scale.y();
    //         scl[2] = scale.z();
    //     }

    //     bool changed = false;
    //     changed |= ImGui::DragFloat3("Position##pos", pos, 0.1f);
    //     changed |= ImGui::DragFloat3("Rotation##rot", rot, 0.5f);
    //     changed |= ImGui::DragFloat3("Scale##scl", scl, 0.1f);

    //     if (changed)
    //     {
    //         RenderWorker::float4x4 transform =
    //             MathWorker::translation(pos[0], pos[1], pos[2]) *
    //             MathWorker::rotation_matrix_yaw_pitch_roll(
    //                 MathWorker::Deg2Rad(rot[1]),
    //                 MathWorker::Deg2Rad(rot[0]),
    //                 MathWorker::Deg2Rad(rot[2])) *
    //             MathWorker::scaling(scl[0], scl[1], scl[2]);

    //         node->TransformToParent(transform);
    //         node->UpdateTransforms();
    //     }

    //     ImGui::TreePop();
    // }
    // ImGui::Spacing();

    // if (ImGui::TreeNodeEx("Mesh Filter", ImGuiTreeNodeFlags_DefaultOpen))
    // {
    //     ImGui::TreePop();
    // }
    // ImGui::Spacing();

    // if (ImGui::CollapsingHeader("Mesh Renderer", ImGuiTreeNodeFlags_DefaultOpen))
    // {
    //     if (ImGui::TreeNode("Materials"))
    //     {
    //         ImGui::TreePop();
    //     }

    //     if (ImGui::TreeNode("Lighting"))
    //     {
    //         ImGui::TreePop();
    //     }

    //     if (ImGui::TreeNode("Lightmaping"))
    //     {
    //         ImGui::TreePop();
    //     }

    //     if (ImGui::TreeNode("Probes"))
    //     {
    //         ImGui::TreePop();
    //     }

    //    if (ImGui::TreeNode("Additional Settings"))
    //     {
    //         ImGui::TreePop();
    //     }
    // }
    // ImGui::Spacing();


}

void EditorInspectorPanel::DrawMaterial(AssertBaseInfo& info)
{
    
}

}
