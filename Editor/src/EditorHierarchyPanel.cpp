#include <editor/EditorHierarchyPanel.h>
#include <editor/EditorManager.h>
#include <base/Context.h>
#include <world/World.h>
#include <render/Renderable.h>
#include <render/RenderableHelper.h>
#include <render/Mesh.h>
#include <common/Util.h>
#include <game/GameContext.h>
#include <Manager/DataManager.h>

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

namespace EditorWorker
{
namespace
{
    std::string ToLowerAscii(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch)
        {
            return static_cast<char>(std::tolower(ch));
        });
        return value;
    }

    MeshPart const* FindMeshPart(std::string const& mesh_name, PrefabData const* npc)
    {
        if (!npc)
        {
            return nullptr;
        }

        std::string const mesh_name_lower = ToLowerAscii(mesh_name);
        for (ModelData const& component : npc->components)
        {
            if (!component.model)
            {
                continue;
            }

            for (MeshPart const& part : component.model->parts)
            {
                if (!part.name.empty() && mesh_name_lower.find(ToLowerAscii(part.name)) != std::string::npos)
                {
                    return &part;
                }
            }
        }
        return nullptr;
    }

    void CollectStaticMeshes(RenderWorker::SceneNode const& node, std::vector<RenderWorker::Renderable const*>& meshes)
    {
        node.ForEachComponentOfType<RenderWorker::RenderableComponent>(
            [&meshes](RenderWorker::RenderableComponent& component)
            {
                RenderWorker::Renderable& renderable = component.BoundRenderable();
                if (dynamic_cast<RenderWorker::StaticMesh*>(&renderable))
                {
                    meshes.push_back(&renderable);
                }
            });

        for (auto const& child : node.Children())
        {
            CollectStaticMeshes(*child, meshes);
        }
    }

    void RenderModelMeshes(
        RenderWorker::SceneNode const& node, PrefabData const* npc, EditorManager& editor)
    {
        std::vector<RenderWorker::Renderable const*> meshes;
        CollectStaticMeshes(node, meshes);
        if (meshes.empty())
        {
            return;
        }

        for (size_t mesh_index = 0; mesh_index < meshes.size(); ++mesh_index)
        {
            std::string mesh_name;
            CommonWorker::Convert(mesh_name, meshes[mesh_index]->Name());
            if (MeshPart const* part = FindMeshPart(mesh_name, npc))
            {
                mesh_name = part->name;
            }
            if (mesh_name.empty())
            {
                mesh_name = "Mesh " + std::to_string(mesh_index);
            }

            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
            if (editor.IsHierarchyItemSelected(&node, mesh_name))
            {
                flags |= ImGuiTreeNodeFlags_Selected;
            }

            ImGui::PushID(static_cast<int>(mesh_index));
            ImGui::TreeNodeEx(mesh_name.c_str(), flags);
            if (ImGui::IsItemClicked())
            {
                editor.SetSelectedSceneNode(&node, mesh_name);
            }
            ImGui::PopID();
        }
    }
}

EditorHierarchyPanel::EditorHierarchyPanel()
{
    
}

EditorHierarchyPanel::~EditorHierarchyPanel()
{
    
}

void EditorHierarchyPanel::OnRender(const EditorSetting& setting)
{
    // 面板大小和位置
    ImGui::SetNextWindowPos(ImVec2(0, (float)setting.mainBarHeight));
    ImGui::SetNextWindowSize(ImVec2((float)setting.hierarchyWidth, (float)setting.hierarchyHeight));

    auto& editor = CommonWorker::checked_cast<EditorManager&>(Context::Instance().AppInstance());

    // 设置面板具体内容
    if (ImGui::Begin("Hierarchy", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
    {
        int i = 0;
        for( const auto& node : Context::Instance().WorldInstance().SceneRootNode().Children() )
        {
            if (i == 0)
                ImGui::SetNextItemOpen(true, ImGuiCond_Once);

            ImGui::PushID(i);
            std::string node_name;
            CommonWorker::Convert(node_name, node->Name());
            PrefabData const* npc = GameContext::Instance().DataManagerInstance().FindNpcByName(node_name);

            ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
            if (editor.IsHierarchyItemSelected(node.get(), {}))
            {
                node_flags |= ImGuiTreeNodeFlags_Selected;
            }

            bool const open = ImGui::TreeNodeEx(node_name.c_str(), node_flags);
            if (ImGui::IsItemClicked())
            {
                editor.SetSelectedSceneNode(node.get(), {});
            }

            if (open)
            {
                RenderModelMeshes(*node, npc, editor);
                ImGui::TreePop();
            }
            ImGui::PopID();
            i++;
        }

        // 窗口空白右键菜单
        auto& root_node = RenderWorker::Context::Instance().WorldInstance().SceneRootNode();
        RenderWorker::Color DefaultColor(1.0f, 0.0f, 0.0f, 1.0f);
        if (ImGui::BeginPopupContextWindow())
        {
            if (ImGui::BeginMenu("3D Object"))
            {
                if( ImGui::MenuItem("Cube") )
                {
                    auto RenderableBox = CommonWorker::MakeSharedPtr<RenderWorker::SceneNode>(
                    CommonWorker::MakeSharedPtr<RenderWorker::RenderableComponent>(
                        CommonWorker::MakeSharedPtr<RenderWorker::RenderableBox>(1.0f, 1.0f, 1.0f, DefaultColor)),
                            L"Cube", RenderWorker::SceneNode::SOA_Cullable);
                    RenderableBox->TransformToParent(MathWorker::translation(0.0f, 0.0f, 0.0f));
                    root_node.AddChild(RenderableBox);
                }
                if(ImGui::MenuItem("Sphere"))
                {
                    auto RenderSphere = CommonWorker::MakeSharedPtr<RenderWorker::SceneNode>(
                        CommonWorker::MakeSharedPtr<RenderWorker::RenderableComponent>(
                            CommonWorker::MakeSharedPtr<RenderWorker::RenderableSphere>(0.5f, 40, 20, DefaultColor)),
                                L"Sphere", RenderWorker::SceneNode::SOA_Cullable);
                    RenderSphere->TransformToParent(MathWorker::translation(0.0f, 0.0f, 0.0f));
                    root_node.AddChild(RenderSphere);
                }
                if(ImGui::MenuItem("Capsule"))
                {

                }
                if(ImGui::MenuItem("Cylinder"))
                {

                }
                if(ImGui::MenuItem("Plane"))
                {
     
                }
                if(ImGui::MenuItem("Quad"))
                {

                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("2D Object"))
            {
                ImGui::MenuItem("Rectangle");
                ImGui::MenuItem("Circle");
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Light"))
            {
                ImGui::MenuItem("Directional Light");
                ImGui::MenuItem("Point Light");
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Audio"))
            {
                ImGui::MenuItem("Sound Effect");
                ImGui::MenuItem("Background Music");
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("UI"))
            {
                ImGui::MenuItem("Button");
                ImGui::MenuItem("Text");
                ImGui::EndMenu();
            }
            ImGui::EndPopup();
        }
    }
    ImGui::End();
}

void EditorHierarchyPanel::OnResize()
{
    
}

}
