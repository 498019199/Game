#include <editor/EditorProjectPanel.h>
#include <editor/EditorManagerD3D11.h>

#include <render/Texture.h>
#include <render/RenderFactory.h>

#include <unordered_set>
#include <string>
#include <filesystem>


namespace EditorWorker
{
using namespace RenderWorker;

// 在Project面板中屏蔽的文件类型
std::unordered_set<std::string> ignoreExtensions = 
{ 
    // 生成的HLSL代码
    ".hlsl",
    // 预编译的D3D12 Shader
    ".fxc",
    // 生成的GLSL代码
    ".vert", ".frag", ".geom",
    // 预编译的Vulkan Shader
    ".spv",
};

EditorProjectPanel::EditorProjectPanel()
{
    ext_type_map_.insert(std::make_pair<std::string, AssetType>("",          AssetType::Folder            ));
    ext_type_map_.insert(std::make_pair<std::string, AssetType>(".mat",      AssetType::Material          ));
    ext_type_map_.insert(std::make_pair<std::string, AssetType>(".drmat",    AssetType::DeferredMaterial  ));
    ext_type_map_.insert(std::make_pair<std::string, AssetType>(".rtmat",    AssetType::RayTracingMaterial));
    ext_type_map_.insert(std::make_pair<std::string, AssetType>(".prefab",   AssetType::Prefab            ));
    ext_type_map_.insert(std::make_pair<std::string, AssetType>(".lua",      AssetType::Script            ));
    ext_type_map_.insert(std::make_pair<std::string, AssetType>(".shader",   AssetType::Shader            ));
    ext_type_map_.insert(std::make_pair<std::string, AssetType>(".dxr",      AssetType::RayTracingShader  ));
    ext_type_map_.insert(std::make_pair<std::string, AssetType>(".vkr",      AssetType::RayTracingShader  ));
    ext_type_map_.insert(std::make_pair<std::string, AssetType>(".png",      AssetType::Texture           ));
    ext_type_map_.insert(std::make_pair<std::string, AssetType>(".tga",      AssetType::Texture           ));
    ext_type_map_.insert(std::make_pair<std::string, AssetType>(".jpg",      AssetType::Texture           ));
    ext_type_map_.insert(std::make_pair<std::string, AssetType>(".scene",    AssetType::Scene             ));
    ext_type_map_.insert(std::make_pair<std::string, AssetType>(".obj",      AssetType::Model             ));
    ext_type_map_.insert(std::make_pair<std::string, AssetType>(".fbx",      AssetType::Model             ));
    ext_type_map_.insert(std::make_pair<std::string, AssetType>(".wav",      AssetType::Audio             ));
    ext_type_map_.insert(std::make_pair<std::string, AssetType>(".ogg",      AssetType::Audio             ));
    ext_type_map_.insert(std::make_pair<std::string, AssetType>(".txt",      AssetType::Text              ));

    auto& rf = Context::Instance().RenderFactoryInstance();
    fileIcons_[(int)AssetType::Other]              = rf.MakeTextureSrv(SyncLoadTexture("EditorAssets/icons/other.png",   EAH_GPU_Read | EAH_Immutable));
    fileIcons_[(int)AssetType::Folder]             = rf.MakeTextureSrv(SyncLoadTexture("EditorAssets/icons/folder.png",  EAH_GPU_Read | EAH_Immutable));
    fileIcons_[(int)AssetType::Material]           = rf.MakeTextureSrv(SyncLoadTexture("EditorAssets/icons/material.png",EAH_GPU_Read | EAH_Immutable));
    fileIcons_[(int)AssetType::DeferredMaterial]   = rf.MakeTextureSrv(SyncLoadTexture("EditorAssets/icons/material.png",EAH_GPU_Read | EAH_Immutable));
    fileIcons_[(int)AssetType::RayTracingMaterial] = rf.MakeTextureSrv(SyncLoadTexture("EditorAssets/icons/material.png",EAH_GPU_Read | EAH_Immutable));
    fileIcons_[(int)AssetType::Prefab]             = rf.MakeTextureSrv(SyncLoadTexture("EditorAssets/icons/prefab.png",  EAH_GPU_Read | EAH_Immutable));
    fileIcons_[(int)AssetType::Script]             = rf.MakeTextureSrv(SyncLoadTexture("EditorAssets/icons/lua.png",     EAH_GPU_Read | EAH_Immutable));
    fileIcons_[(int)AssetType::Shader]             = rf.MakeTextureSrv(SyncLoadTexture("EditorAssets/icons/shader.png",  EAH_GPU_Read | EAH_Immutable));
    fileIcons_[(int)AssetType::Texture]            = rf.MakeTextureSrv(SyncLoadTexture("EditorAssets/icons/texture.png", EAH_GPU_Read | EAH_Immutable));
    fileIcons_[(int)AssetType::Scene]              = rf.MakeTextureSrv(SyncLoadTexture("EditorAssets/icons/scene.png",   EAH_GPU_Read | EAH_Immutable));
    fileIcons_[(int)AssetType::Model]              = rf.MakeTextureSrv(SyncLoadTexture("EditorAssets/icons/model.png",   EAH_GPU_Read | EAH_Immutable));
    fileIcons_[(int)AssetType::RayTracingShader]   = rf.MakeTextureSrv(SyncLoadTexture("EditorAssets/icons/raytrace.png",EAH_GPU_Read | EAH_Immutable));
    fileIcons_[(int)AssetType::Audio]              = rf.MakeTextureSrv(SyncLoadTexture("EditorAssets/icons/audio.png",   EAH_GPU_Read | EAH_Immutable));
    fileIcons_[(int)AssetType::Text]               = rf.MakeTextureSrv(SyncLoadTexture("EditorAssets/icons/text.png",    EAH_GPU_Read | EAH_Immutable));

    root_ =  CommonWorker::MakeSharedPtr<EditorAssetNode>();
    root_->parent = nullptr;
    const std::string& LocalPath = RenderWorker::Context::Instance().ResLoaderInstance().LocalFolder();
    root_->path = (LocalPath + "../../Assets");
    root_->name = "Assets";
    root_->extension = "";
    root_->type = AssetType::Folder;
    GetChildren(root_);
    cur_ = root_;
}

EditorProjectPanel::~EditorProjectPanel()
{
    
}

void EditorProjectPanel::OnRender(const EditorSetting& setting)
{
    // 面板大小和位置
    ImGui::SetNextWindowPos(ImVec2(0, (float)setting.mainBarHeight + (float)setting.hierarchyHeight));
    ImGui::SetNextWindowSize(ImVec2((float)setting.projectWidth, (float)setting.projectHeight));

    // 设置面板具体内容
    ImGui::Begin("Peoject", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
    {
        // 记录一下按钮原本颜色
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4 btnColor = style.Colors[ImGuiCol_Button];
        ImVec4 selectBtnColor = ImVec4(btnColor.x - 0.1f, btnColor.y - 0.1f, btnColor.z - 0.1f, 1.0f);
        ImVec4 textColor = style.Colors[ImGuiCol_Text];
		ImVec4 selectTextColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        // 当前路径节点，如果当前节点不是文件夹，就用当前节点的父节点
        auto curPathNode = cur_->type == AssetType::Folder ? cur_ : cur_->parent;

        // 绘制路径条
        ImGui::PushStyleColor(ImGuiCol_Button, style.Colors[ImGuiCol_WindowBg]);
        auto tmpPathNode = curPathNode;
        std::vector<EditorAssetNodePtr> pathNodes;
        while ( tmpPathNode )
        {
            pathNodes.push_back(tmpPathNode);
            tmpPathNode = tmpPathNode->parent;
        }
        // 倒叙绘制，从root到当前位置
        for (auto i = pathNodes.size(); i > 0; i--)
        {
            ImGui::SameLine(); 
            if (ImGui::SmallButton(pathNodes[i - 1]->name.c_str()))
            {
                SetCurNode( pathNodes[i - 1] );
                // 切换路径的时候刷新选中状态
                selected_ = -1;
            }
            if (i > 1)
            {
                ImGui::SameLine();
                ImGui::Text(">");
            }
        }
        ImGui::PopStyleColor(1);
        ImGui::Separator();

        // 当前路径文件数量
        std::size_t childNum = curPathNode->children.size();
        // 当前窗口的x最大值(右边界位置)
        float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
        // 记录当前绘制的一排文件名
        std::vector<std::string> fileNames;
        // 当前绘制的文件在当前这一行里是第几个
        int rowIdx = 0;
        // 记录当前选中的文件在当前这一行里是第几个
        int curRowIdx = -1;
        for (size_t i = 0; i < childNum; i++)
        {
            auto node = curPathNode->children[i];
            if (i == selected_)
            {
                curRowIdx = rowIdx;
                ImGui::PushStyleColor(ImGuiCol_Button, selectBtnColor);
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Button, btnColor);
            }

            // 无论是否点击都必须PopStyleColor，所以没有直接写在if中
            std::string label = "##File" + std::to_string(i);
            auto icon = fileIcons_[(int)node->type]->GetShaderResourceView();
            bool click = ImGui::ImageButton(label.c_str(), (ImTextureID)(intptr_t)icon, iconSize);
            ImGui::PopStyleColor(1);
            if (click)
            {
                selected_ = static_cast<int>(i);
                SetCurNode( node );
                if (node->type == AssetType::Folder)
                {
                    // 切换路径的时候刷新选中状态
                    selected_ = -1;
                    break;
                }
            }
            fileNames.push_back(node->name);

            // 计算是否换行
            float last_button_x2 = ImGui::GetItemRectMax().x;
            float next_button_x2 = last_button_x2 + style.ItemSpacing.x; //+ iconSize.x;
            if (i + 1 < childNum && next_button_x2 < window_visible_x2)
            {
                rowIdx++;
                ImGui::SameLine();
            }
            else
            {
                rowIdx = 0;
                // 绘制下一行文件前，先把这一行的文件名绘制出来
                for (size_t j = 0; j < fileNames.size(); j++)
                {
                    if (j > 0)
                        ImGui::SameLine();

                    if (curRowIdx == j)
                    {
                        ImGui::PushStyleColor(ImGuiCol_Text, selectTextColor);
                        ImGui::PushStyleColor(ImGuiCol_Button, selectBtnColor);
                    }
                    else
                    {
                        ImGui::PushStyleColor(ImGuiCol_Text, textColor);
                        ImGui::PushStyleColor(ImGuiCol_Button, style.Colors[ImGuiCol_WindowBg]);
                    }

                    ImGui::Button(fileNames[j].c_str(), nameSize);
                    ImGui::PopStyleColor(2);
                }
                fileNames.clear();
                curRowIdx = -1;
            }
        }
    }
    ImGui::End();
}

AssetType EditorProjectPanel::GetAssetType(const std::string& extension)
{
    auto it = ext_type_map_.find( extension );
    if (it == ext_type_map_.end())
        return AssetType::Other;
    else
        return it->second;
}

void EditorProjectPanel::SetCurNode(const EditorAssetNodePtr& node)
{
    cur_ = node;

    auto& d3d_editor = checked_cast<EditorManagerD3D11&>(Context::Instance().AppInstance());
    d3d_editor.SetSelectedAssert( node );
}

void EditorProjectPanel::GetChildren(const EditorAssetNodePtr& node)
{
    for (const auto& entry : std::filesystem::directory_iterator(node->path))
    {
        std::string extension = entry.path().filename().extension().string();
        // 如果是忽略的文件类型，就跳过
        if (ignoreExtensions.find(extension) != ignoreExtensions.end())
        {    
            continue;
        }

        EditorAssetNodePtr child =  CommonWorker::MakeSharedPtr<EditorAssetNode>();
        child->parent = node;
        child->path = entry.path().string();
        child->name = entry.path().stem().string();
        child->extension = extension;

        if (entry.is_directory())
        {
            child->size = 0;
            child->type = AssetType::Folder;
        }
        else
        {
            child->size = static_cast<uint32_t>(entry.file_size());
            child->type = GetAssetType(child->extension);
        }
        node->children.push_back(child);

        // 排序
        std::sort(node->children.begin(), node->children.end(), 
            [](const EditorAssetNodePtr& a, const EditorAssetNodePtr& b) 
            { 
                // 文件夹排在前面
                if (a->type == AssetType::Folder && b->type != AssetType::Folder)
                    return true;
                if (a->type != AssetType::Folder && b->type == AssetType::Folder)
                    return false;
                // 按名字排序
                return a->name < b->name;
            }
        );

        if (child->type == AssetType::Folder)
        {    
            GetChildren(child);
        }
    }
}

void EditorProjectPanel::OnResize()
{
    
}

}