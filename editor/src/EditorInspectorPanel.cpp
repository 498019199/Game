#include <editor/EditorInspectorPanel.h>
#include <editor/EditorManager.h>
#include <editor/EditorProjectPanel.h>
#include <editor/EditorAssetScanner.h>
#include <editor/EditorTransform.h>
#include <exception>
#include <algorithm>
#include <cmath>
#include <cstdio>

#include <base/ZEngine.h>
#include <common/Util.h>

#include <render/RenderFactory.h>
#include <base/AudioFactory.h>

namespace EditorWorker
{
namespace
{
void DrawParticleInspector(EditorManager& editor)
{
    auto node = editor.SelectedTransformNode();
    auto particles = editor.Scene().FindParticleEffect(node.get());
    if (!particles) return;
    ImGui::SeparatorText("Particle Effect");
    std::string name;
    CommonWorker::Convert(name, node->Name());
    char name_buffer[256];
    std::snprintf(name_buffer, sizeof(name_buffer), "%s", name.c_str());
    if (ImGui::InputText("Name", name_buffer, sizeof(name_buffer)) && name_buffer[0]) {
        std::wstring value;
        CommonWorker::Convert(value, std::string_view(name_buffer));
        node->Name(value);
    }
    if (ImGui::Button("Frame effect")) editor.FrameSelection();
    ImGui::SameLine();
    if (ImGui::Button("Delete effect")) {
        editor.ClearAssetSelection();
        editor.Scene().RemoveParticleEffect(node.get());
        return;
    }
    ImGui::TextWrapped("T: move  R: rotate  S: scale in Scene view.");
    auto transform = node->TransformToParent();
    float position[3] = {transform(3,0), transform(3,1), transform(3,2)};
    if (ImGui::DragFloat3("Position", position, .05f)
        && std::isfinite(position[0]) && std::isfinite(position[1]) && std::isfinite(position[2])) {
        for (size_t i = 0; i < 3; ++i) transform(3,i) = position[i];
        node->TransformToParent(transform);
        node->UpdateTransforms();
    }
    if (ImGui::Button(particles->IsPlaying() ? "Pause" : "Play")) {
        if (particles->IsPlaying()) particles->Pause(); else particles->Play();
    }
    ImGui::SameLine(); if (ImGui::Button("Restart")) particles->Restart();
    ImGui::SameLine(); if (ImGui::Button("Stop")) particles->Stop();
    ImGui::Text("Alive: %zu / %u", particles->Particles().size(), particles->Settings().max_particles);
    ImGui::TextDisabled("%s", particles->IsPlaying() ? (particles->IsEmitting() ? "Playing" : "Finishing live particles") : "Paused / stopped");
    if (ImGui::BeginCombo("Apply preset", "Select preset")) {
        char const* labels[] = {"Fire", "Smoke", "Sparks", "Explosion", "Heat haze"};
        for (int i = 0; i < 5; ++i) {
            char const* label = labels[i];
            if (ImGui::Selectable(label)) particles->Settings(ParticleSettings::ForPreset(static_cast<ParticlePreset>(i)));
        }
        ImGui::EndCombo();
    }
    auto settings = particles->Settings();
    bool changed = false;
    if (ImGui::CollapsingHeader("Emission", ImGuiTreeNodeFlags_DefaultOpen)) {
        int space = settings.simulation_space == ParticleSimulationSpace::Local ? 0 : 1;
        changed |= ImGui::Combo("Simulation space", &space, "Local (follow emitter)\0World (leave particles behind)\0");
        settings.simulation_space = space == 0 ? ParticleSimulationSpace::Local : ParticleSimulationSpace::World;
        ImGui::TextWrapped(space == 0 ? "Moving the emitter also moves live particles." :
            "Only new particles follow the emitter. Existing particles keep moving in world space.");
        int capacity = static_cast<int>(settings.max_particles);
        changed |= ImGui::DragInt("Max particles", &capacity, 1, 1, 16384, "%d", ImGuiSliderFlags_AlwaysClamp);
        settings.max_particles = static_cast<uint32_t>(capacity);
        changed |= ImGui::DragFloat("Rate / second", &settings.emission_rate, 1, 0, 50000, "%.1f", ImGuiSliderFlags_AlwaysClamp);
        int burst = static_cast<int>(settings.burst_count);
        changed |= ImGui::DragInt("Burst count", &burst, 1, 0, 16384, "%d", ImGuiSliderFlags_AlwaysClamp);
        settings.burst_count = static_cast<uint32_t>(burst);
        ImGui::BeginDisabled(!particles->Enabled() || !particles->IsPlaying() || settings.burst_count == 0 || changed);
        if (ImGui::Button("Emit burst now")) particles->EmitBurst(settings.burst_count);
        ImGui::EndDisabled();
        changed |= ImGui::Checkbox("Looping", &settings.looping);
        changed |= ImGui::DragFloat(settings.looping ? "Cycle duration" : "Emission duration",
            &settings.duration, .05f, .01f, 3600, "%.2f s", ImGuiSliderFlags_AlwaysClamp);
        if (settings.burst_count > 0)
            ImGui::TextWrapped(settings.looping ? "Bursts emit at the start of each cycle. Rate adds continuous emission." :
                "One burst emits on start. Restart replays it; remaining particles finish after emission ends.");
        changed |= ImGui::DragFloat("Lifetime min", &settings.lifetime_min, .01f, .01f, settings.lifetime_max, "%.2f s", ImGuiSliderFlags_AlwaysClamp);
        changed |= ImGui::DragFloat("Lifetime max", &settings.lifetime_max, .01f, settings.lifetime_min, 60, "%.2f s", ImGuiSliderFlags_AlwaysClamp);
        changed |= ImGui::InputScalar("Random seed", ImGuiDataType_U32, &settings.seed);
    }
    if (ImGui::CollapsingHeader("Motion")) {
        changed |= ImGui::DragFloat("Speed min", &settings.speed_min, .05f, 0, settings.speed_max, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        changed |= ImGui::DragFloat("Speed max", &settings.speed_max, .05f, settings.speed_min, 1000, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        float spread = MathWorker::Rad2Deg(settings.spread);
        if (ImGui::SliderFloat("Cone angle", &spread, 0, 180, "%.1f deg")) {
            settings.spread = MathWorker::Deg2Rad(spread); changed = true;
        }
        changed |= ImGui::DragFloat("Emitter radius", &settings.radius, .01f, 0, 1000, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        changed |= ImGui::DragFloat3("Direction", settings.direction.data(), .05f, -1, 1, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        changed |= ImGui::DragFloat3("Acceleration", settings.acceleration.data(), .05f, -1000, 1000, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        float angular_min = MathWorker::Rad2Deg(settings.angular_speed_min), angular_max = MathWorker::Rad2Deg(settings.angular_speed_max);
        if (ImGui::DragFloat("Spin min", &angular_min, 1, -3600, angular_max, "%.1f deg/s", ImGuiSliderFlags_AlwaysClamp)) {
            settings.angular_speed_min = MathWorker::Deg2Rad(angular_min); changed = true;
        }
        if (ImGui::DragFloat("Spin max", &angular_max, 1, angular_min, 3600, "%.1f deg/s", ImGuiSliderFlags_AlwaysClamp)) {
            settings.angular_speed_max = MathWorker::Deg2Rad(angular_max); changed = true;
        }
    }
    if (ImGui::CollapsingHeader("Appearance", ImGuiTreeNodeFlags_DefaultOpen)) {
        changed |= ImGui::DragFloat("Start size", &settings.start_size, .01f, .001f, 100, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        changed |= ImGui::DragFloat("End size", &settings.end_size, .01f, .001f, 100, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        float rotation_min = MathWorker::Rad2Deg(settings.rotation_min), rotation_max = MathWorker::Rad2Deg(settings.rotation_max);
        if (ImGui::DragFloat("Rotation min", &rotation_min, 1, -360, rotation_max, "%.1f deg", ImGuiSliderFlags_AlwaysClamp)) {
            settings.rotation_min = MathWorker::Deg2Rad(rotation_min); changed = true;
        }
        if (ImGui::DragFloat("Rotation max", &rotation_max, 1, rotation_min, 360, "%.1f deg", ImGuiSliderFlags_AlwaysClamp)) {
            settings.rotation_max = MathWorker::Deg2Rad(rotation_max); changed = true;
        }
        changed |= ImGui::DragFloat("Velocity stretch", &settings.velocity_stretch, .01f, 0, 10, "%.2f s", ImGuiSliderFlags_AlwaysClamp);
        if (settings.velocity_stretch > 0)
            ImGui::TextWrapped("Elongates sprites along screen-space motion. Stretch is the trail length per unit of speed.");
        changed |= ImGui::ColorEdit4("Start color", settings.start_color.data(), ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);
        changed |= ImGui::ColorEdit4("End color", settings.end_color.data(), ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);
        int blend = settings.blend == ParticleBlendMode::Alpha ? 0 : settings.blend == ParticleBlendMode::Additive ? 1 : 2;
        changed |= ImGui::Combo("Blending", &blend, "Alpha (smoke)\0Additive (glow)\0Distortion (heat haze)\0");
        settings.blend = blend == 0 ? ParticleBlendMode::Alpha : blend == 1 ? ParticleBlendMode::Additive : ParticleBlendMode::Distortion;
    }
    if (ImGui::CollapsingHeader("Light and refraction", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (settings.blend == ParticleBlendMode::Distortion) {
            changed |= ImGui::DragFloat("Distortion", &settings.distortion_strength, .25f, 0, 64, "%.1f px", ImGuiSliderFlags_AlwaysClamp);
            ImGui::TextWrapped("Refracts the scene behind each particle. Place it in front of visible geometry; a flat background has no detail to distort.");
        } else {
            changed |= ImGui::DragFloat("Emissive intensity", &settings.emissive_intensity, .05f, 0, 16, "%.2f", ImGuiSliderFlags_AlwaysClamp);
            changed |= ImGui::DragFloat("Glow strength", &settings.glow_strength, .05f, 0, 4, "%.2f", ImGuiSliderFlags_AlwaysClamp);
            ImGui::TextWrapped("Glow adds a soft halo to the particle.");
        }
        changed |= ImGui::DragFloat("Flicker frequency", &settings.flicker_frequency, .1f, 0, 60, "%.1f Hz", ImGuiSliderFlags_AlwaysClamp);
        changed |= ImGui::SliderFloat("Flicker amount", &settings.flicker_amount, 0, 1, "%.2f");
    }
    if (changed) particles->Settings(settings);
    ImGui::TextWrapped("Parameter changes restart the preview. Save Scene stores the emitter settings and transform.");
}
}

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
            DrawParticleInspector(editor);
            if (ImGui::BeginCombo("Materials", "Open documents"))
            {
                for (auto const& [path, doc] : editor.Materials().Documents())
                    if (ImGui::Selectable((path + (doc->Dirty() ? " *" : "")).c_str())) editor.SelectMaterialDocument(doc);
                ImGui::EndCombo();
            }
            auto camera_node = editor.Scene().SceneCameraNode();
            if (camera_node && editor.IsHierarchyItemSelected(camera_node.get())) {
                auto& camera = *editor.Scene().SceneCamera();
                ImGui::SeparatorText("Scene Camera");
                if (ImGui::Button("Frame camera")) editor.FrameSelection();
                ImGui::TextWrapped("T: move  R: rotate. Select Scene to see the camera frustum.");
                auto matrix = camera_node->TransformToParent();
                float position[3] = {matrix(3,0),matrix(3,1),matrix(3,2)};
                if (ImGui::DragFloat3("Position", position, .1f)
                    && std::isfinite(position[0]) && std::isfinite(position[1]) && std::isfinite(position[2])) {
                    for (size_t i=0;i<3;++i) matrix(3,i) = position[i];
                    camera_node->TransformToParent(matrix); camera_node->UpdateTransforms(); camera.Dirty();
                }
                float fov = MathWorker::Rad2Deg(camera.FOV()), aspect = camera.Aspect();
                float near_plane = camera.NearPlane(), far_plane = camera.FarPlane();
                bool changed = ImGui::DragFloat("FOV", &fov, .25f, 1.f, 179.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
                changed |= ImGui::DragFloat("Aspect", &aspect, .01f, .1f, 10.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
                changed |= ImGui::DragFloat("Near", &near_plane, .01f, .001f, far_plane-.001f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
                changed |= ImGui::DragFloat("Far", &far_plane, .25f, near_plane+.001f, 100000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
                if (changed && std::isfinite(fov) && std::isfinite(aspect)
                    && std::isfinite(near_plane) && std::isfinite(far_plane)
                    && fov > 0 && fov < 180 && aspect > 0 && near_plane > 0 && far_plane > near_plane)
                    camera.ProjParams(MathWorker::Deg2Rad(fov), aspect, near_plane, far_plane);
            }
            const auto pAsset = editor.GetSelectedAssert();
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

                    default:
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

    if (ImGui::Button("Frame model")) checked_cast<EditorManager&>(Context::Instance().AppInstance()).FrameModel(model_info.model);
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
        if (ImGui::TreeNodeEx("Materials", ImGuiTreeNodeFlags_DefaultOpen))
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
                ImGui::PushID(static_cast<int>(i));
                ImGui::TextWrapped("%s", label.c_str());
                auto& editor = checked_cast<EditorManager&>(Context::Instance().AppInstance());
                bool scene_model = editor.Scene().ContainsModel(model_info.model);
                auto asset = editor.Scene().MaterialAsset(model_info.model, static_cast<uint32_t>(i));
                ImGui::TextWrapped("Asset: %s", asset.empty() ? "(embedded)" : asset.c_str());
                ImGui::BeginDisabled(!scene_model);
                if (ImGui::BeginCombo("Assign", "Open material documents")) {
                    for (auto const& [path, doc] : editor.Materials().Documents())
                        if (ImGui::Selectable(path.c_str(), path == asset)) {
                            try { editor.Scene().AssignMaterial(model_info.model, static_cast<uint32_t>(i), path, doc->Material().Clone()); model_info.error.clear(); }
                            catch (std::exception const& error) { model_info.error = error.what(); }
                        }
                    ImGui::EndCombo();
                }
                auto selected = editor.PreviewDocument();
                if (selected) {
                    ImGui::TextWrapped("Selected: %s", selected->Path().c_str());
                    if (ImGui::Button("Use selected material")) {
                        try { editor.Scene().AssignMaterial(model_info.model, static_cast<uint32_t>(i), selected->Path(), selected->Material().Clone()); model_info.error.clear(); }
                        catch (std::exception const& error) { model_info.error = error.what(); }
                    }
                }
                ImGui::EndDisabled();
                if (!asset.empty() && ImGui::Button("Edit assigned material")) {
                    try { editor.SelectMaterialDocument(editor.Materials().Open(asset)); }
                    catch (std::exception const& error) { model_info.error = error.what(); }
                }
                if (!scene_model) ImGui::TextWrapped("Select a model in Hierarchy to save its material assignment.");
                if (!model_info.error.empty()) ImGui::TextWrapped("%s", model_info.error.c_str());
                ImGui::Separator(); ImGui::PopID();
            }
            ImGui::TreePop();
        }
    }
    ImGui::Spacing();
}

void EditorInspectorPanel::DrawMaterial(AssertBaseInfo& info)
{
    auto& asset = checked_cast<AssetMaterialInfo&>(info);
    auto& editor = checked_cast<EditorManager&>(Context::Instance().AppInstance());
    auto& service = editor.Materials();
    auto attempt = [&](auto action) {
        try { action(); asset.error.clear(); }
        catch (std::exception const& e) { asset.error = e.what(); }
    };
    if (!asset.error.empty()) ImGui::TextWrapped("Error: %s", asset.error.c_str());
    if (!asset.document)
    {
        ImGui::TextWrapped("Material unavailable: %s", asset.path.c_str());
        if (ImGui::Button("Retry Open")) attempt([&] { asset.document = service.Open(asset.path); });
        return;
    }
    auto doc = asset.document;
    ImGui::TextWrapped("%s%s", doc->Path().c_str(), doc->Dirty() ? " * (unsaved)" : " (saved)");
    ImGui::Text("Template: %s", doc->Material().Template().empty() ? "Legacy" : doc->Material().Template().c_str());
    ImGui::TextWrapped("Preview in the Material tab. Select a scene model to assign this material.");
    editor.PreviewMaterial(doc);
    if (doc->Material().Template().empty()) ImGui::TextWrapped("Legacy rendering is preserved; not every parameter is visible in the legacy shader.");
    if (ImGui::Button("Save")) attempt([&] { service.Save(*doc); });
    ImGui::SameLine(); if (ImGui::Button("Save As")) ImGui::OpenPopup("Save Material As");
    if (ImGui::Button("Revert")) ImGui::OpenPopup("Revert material?");
    ImGui::SameLine(); if (ImGui::Button("Reload Disk")) ImGui::OpenPopup("Reload material?");
    ImGui::SameLine(); if (ImGui::Button("Close")) ImGui::OpenPopup("Close material?");

    if (ImGui::BeginPopupModal("Save Material As", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextUnformatted("New Assets-relative .mat path (existing files are never overwritten)");
        ImGui::InputText("Path", asset.save_as, sizeof(asset.save_as));
        if (!asset.error.empty()) ImGui::TextWrapped("%s", asset.error.c_str());
        if (ImGui::Button("Save new asset")) attempt([&] {
            auto created = service.SaveAs(*doc, asset.save_as);
            editor.SelectMaterialDocument(created);
            editor.assets_refresh_requested = true; editor.assets_select_path = created->Path();
            ImGui::CloseCurrentPopup();
        });
        ImGui::SameLine(); if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    auto confirm = [&](char const* title, auto action) {
        if (ImGui::BeginPopupModal(title, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextUnformatted("Discard current edits? This cannot be undone.");
            if (!asset.error.empty()) ImGui::TextWrapped("%s", asset.error.c_str());
            if (ImGui::Button("Confirm")) attempt([&] { action(); ImGui::CloseCurrentPopup(); });
            ImGui::SameLine(); if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }
    };
    confirm("Revert material?", [&] { doc->Revert(); });
    confirm("Reload material?", [&] { service.Reload(*doc); });
    confirm("Reset parameters?", [&] { doc->ResetParameters(); });
    if (ImGui::BeginPopupModal("Close material?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextUnformatted(doc->Dirty() ? "Save changes before closing?" : "Close this document?");
        if (!asset.error.empty()) ImGui::TextWrapped("%s", asset.error.c_str());
        auto close = [&](MaterialCloseChoice choice) { attempt([&] {
            editor.CloseMaterialDocument(doc, choice);
            ImGui::CloseCurrentPopup();
        }); };
        if (ImGui::Button("Save and Close")) close(MaterialCloseChoice::Save);
        ImGui::SameLine(); if (ImGui::Button("Discard and Close")) close(MaterialCloseChoice::Discard);
        ImGui::SameLine(); if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    ImGui::Separator();
    auto p = doc->Parameters();
    char name[512] = {};
    std::copy_n(p.name.data(), (std::min)(p.name.size(), sizeof(name) - 1), name);
    bool changed = ImGui::InputText("Name", name, sizeof(name));
    if (changed) p.name = name;
    float color[] = {p.albedo[0], p.albedo[1], p.albedo[2], p.albedo[3]};
    if (ImGui::ColorEdit4("Albedo", color, ImGuiColorEditFlags_Float)) { p.albedo = float4(color); changed = true; }
    changed |= ImGui::SliderFloat("Metalness", &p.metalness, 0, 1, "%.3f", ImGuiSliderFlags_AlwaysClamp);
    changed |= ImGui::SliderFloat("Glossiness", &p.glossiness, 0, 1, "%.3f", ImGuiSliderFlags_AlwaysClamp);
    float emission[] = {p.emissive[0], p.emissive[1], p.emissive[2]};
    if (ImGui::DragFloat3("Emissive", emission, 0.01f, 0, 64, "%.3f", ImGuiSliderFlags_AlwaysClamp)) { p.emissive = float3(emission); changed = true; }
    changed |= ImGui::SliderFloat("Normal Scale", &p.normal_scale, 0, 4, "%.3f", ImGuiSliderFlags_AlwaysClamp);
    changed |= ImGui::SliderFloat("Occlusion", &p.occlusion_strength, 0, 1, "%.3f", ImGuiSliderFlags_AlwaysClamp);
    if (changed) attempt([&] { doc->Edit(p); });
    if (ImGui::Button("Reset Parameters")) ImGui::OpenPopup("Reset parameters?");
    ImGui::TextWrapped("Reset preserves name, textures and advanced fields. Emissive: 0..64; normal: 0..4; other values: 0..1.");

    if (ImGui::CollapsingHeader("Textures", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::Button("Refresh Texture List")) attempt([&] {
            asset.texture_choices.clear();
            auto root = std::filesystem::path(Context::Instance().ResLoaderInstance().LocalFolder()) / "../../assets";
            EditorAssetScanner scanner;
            std::vector<std::filesystem::path> pending {root};
            while (!pending.empty()) {
                auto directory = pending.back(); pending.pop_back();
                for (auto const& entry : scanner.ReadDirectory(directory)) {
                    if (entry.directory) pending.push_back(entry.path);
                    else if (EditorMaterialService::IsTexture(entry.path))
                        asset.texture_choices.push_back(MaterialUtf8(entry.path.lexically_relative(root)));
                }
            }
            std::sort(asset.texture_choices.begin(), asset.texture_choices.end());
        });
        char const* labels[] = {"Albedo texture", "Metalness / Glossiness", "Normal texture", "Emissive texture", "Occlusion texture"};
        RenderMaterial::TextureSlot slots[] = {RenderMaterial::TS_Albedo, RenderMaterial::TS_MetalnessGlossiness, RenderMaterial::TS_Normal, RenderMaterial::TS_Emissive, RenderMaterial::TS_Occlusion};
        for (int i = 0; i < 5; ++i) {
            ImGui::PushID(i);
            ImGui::TextUnformatted(labels[i]);
            auto const current = doc->Material().TextureName(slots[i]);
            ImGui::TextWrapped("Current: %s", current.empty() ? "(none)" : current.c_str());
            if (ImGui::BeginCombo("Pick", "Select texture")) {
                for (auto const& path : asset.texture_choices)
                    if (ImGui::Selectable(path.c_str(), path == current)) attempt([&] { service.SetTexture(*doc, slots[i], path); });
                ImGui::EndCombo();
            }
            ImGui::InputText("Path", asset.texture_path[i], sizeof(asset.texture_path[i]));
            if (ImGui::Button("Use Path")) attempt([&] { service.SetTexture(*doc, slots[i], asset.texture_path[i]); });
            ImGui::SameLine(); if (ImGui::Button("Clear")) attempt([&] { service.SetTexture(*doc, slots[i], ""); });
            auto const srv = doc->Material().Texture(slots[i]);
            if (srv) if (auto handle = editor.ImGuiTextureHandle(srv)) ImGui::Image((ImTextureID)(intptr_t)handle, ImVec2(64, 64));
            ImGui::PopID();
        }
    }
}

}
