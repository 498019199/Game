#include <editor/EditorHierarchyPanel.h>
#include <base/Context.h>
#include <world/World.h>
#include <render/RenderableHelper.h>
#include <render/Mesh.h>
#include <common/Util.h>

namespace EditorWorker
{

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

            if (ImGui::TreeNode("", node_name.c_str(), i))
            {
                // ImGui::Text(node_name.c_str());
                // ImGui::SameLine();
                // if (ImGui::SmallButton("button")) {}
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
