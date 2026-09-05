#include <editor/EditorInspectorPanel.h>
#include <editor/EditorManager.h>
#include <editor/EditorProjectPanel.h>
#include <editor/EditorTransform.h>
#include <exception>

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
            auto& editor = checked_cast<EditorManager&>(Context::Instance().AppInstance());
            const auto& pAsset = editor.GetSelectedAssert();
            if( pAsset )
            {
                switch (editor.GetAssertType())
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
    const auto* shader_info = dynamic_cast<const AssetShaderInfo*>(&info);
    if (!shader_info)
    {
        ImGui::TextUnformatted("Invalid shader asset information.");
        return;
    }
    std::string title = shader_info->name + " (Shader)";
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (!ImGui::CollapsingHeader(title.c_str()))
    {    
        return;
    }

    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextUnformatted(shader_info->preview.c_str());
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
    auto& editor = checked_cast<EditorManager&>(Context::Instance().AppInstance());
    auto srv = editor.ImGuiTextureHandle(srv_ptr);
    if (srv)
    {
        ImGui::Image((ImTextureID)(intptr_t)srv, ImVec2((float)width, (float)height));
    }
}

void EditorInspectorPanel::DrawAudio(AssertBaseInfo& info)
{
    auto& audio_info = checked_cast<AssetAudioInfo&>(info);
    if (!audio_info.error.empty())
    {
        ImGui::TextWrapped("%s", audio_info.error.c_str());
        return;
    }
    if (!audio_info.audio_buff_)
    {
        ImGui::TextUnformatted("Audio is not loaded.");
        return;
    }
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
            ImGui::Text("%s", "AF_Stereo16"); break;
        default:
            ImGui::Text("%s", "AF_Unknown");
    }
    
    ImGui::Text("Size:");
    ImGui::SameLine(120);
    ImGui::Text("%zu", audio_info.decoded_size);

    // One buffer per selection, owned here rather than by a global numeric ID.
    if (!audio_info.preview_buffer_)
    {
        try
        {
            auto& af = Context::Instance().AudioFactoryInstance();
            audio_info.preview_buffer_ = af.MakeMusicBuffer(audio_info.audio_buff_, 3);
            if (!audio_info.preview_buffer_)
                audio_info.error = "Audio preview buffer creation failed.";
        }
        catch (std::exception const& e)
        {
            audio_info.error = e.what();
        }
        if (!audio_info.preview_buffer_)
        {
            ImGui::TextWrapped("%s", audio_info.error.c_str());
            return;
        }
    }


    ImGui::SetCursorPosX(80);
    if (ImGui::Button("Play", ImVec2(60.0f, 20.0f)))
    {
        audio_info.preview_buffer_->Play(false);
    }
    
    ImGui::SetCursorPosX(80);
    if (ImGui::Button("Stop", ImVec2(60.0f, 20.0f)))
    {
        audio_info.preview_buffer_->Stop();
    }
}

void EditorInspectorPanel::DrawModel(AssertBaseInfo& info)
{
    auto& model_info = checked_cast<AssetModelInfo&>(info);
    if (!model_info.model)
    {
        ImGui::TextUnformatted("Model is not loaded.");
        return;
    }

    ImGui::Text("Name:");
    ImGui::SameLine(120);
    ImGui::Text("%s", model_info.name.c_str());

    ImGui::Text("Meshes:");
    ImGui::SameLine(120);
    ImGui::Text("%u", model_info.model->NumMeshes());

    if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_DefaultOpen))
    {
        auto node = model_info.model->RootNode();
        auto const original = node->TransformToParent();
        RenderWorker::float3 scale;
        bool nonsingular = true;
        for (size_t row = 0; row < 3; ++row)
        {
            scale[row] = std::sqrt(original(row, 0) * original(row, 0) +
                original(row, 1) * original(row, 1) + original(row, 2) * original(row, 2));
            nonsingular &= scale[row] > 1e-6f;
        }
        RenderWorker::float3 angles(0, 0, 0);
        if (nonsingular)
        {
            RenderWorker::quater rotation;
            RenderWorker::float3 translation;
            MathWorker::decompose(scale, rotation, translation, original);
            angles = InspectorEulerDegrees(MathWorker::to_matrix(rotation));
        }
        float pos[3] = {original(3, 0), original(3, 1), original(3, 2)};
        float rot[3] = {angles.x(), angles.y(), angles.z()};
        float scl[3] = {scale.x(), scale.y(), scale.z()};

        bool const position_changed = ImGui::DragFloat3("Position##pos", pos, 0.1f);
        ImGui::BeginDisabled(!nonsingular);
        bool const rotation_changed = ImGui::DragFloat3("Rotation##rot", rot, 0.5f);
        bool const scale_changed = ImGui::DragFloat3("Scale##scl", scl, 0.1f, 0.001f, 10000.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::EndDisabled();
        if (!nonsingular)
            ImGui::TextUnformatted("Rotation/scale editing requires non-zero scale.");

        if (position_changed || rotation_changed || scale_changed)
        {
            auto transform = InspectorTransform(original, RenderWorker::float3(pos),
                RenderWorker::float3(rot), RenderWorker::float3(scl), scale, rotation_changed, scale_changed);
            node->TransformToParent(transform);
            node->UpdateTransforms();
        }

        ImGui::TreePop();
    }
    ImGui::Spacing();

    if (ImGui::TreeNodeEx("Mesh Filter", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::TreePop();
    }
    ImGui::Spacing();

    if (ImGui::CollapsingHeader("Mesh Renderer", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::TreeNode("Materials"))
        {
            for (size_t i = 0; i < model_info.model->NumMaterials(); ++i)
            {
                RenderMaterialPtr const& mtl = model_info.model->GetMaterial(static_cast<int32_t>(i));
                std::string label = "Material " + std::to_string(i);
                if (mtl && !mtl->Name().empty())
                {
                    label += " (";
                    label += mtl->Name();
                    label += ")";
                }
                ImGui::BulletText("%s", label.c_str());
            }
            ImGui::TreePop();
        }
    }
    ImGui::Spacing();
}

void EditorInspectorPanel::DrawMaterial(AssertBaseInfo& info)
{
    
}

}
