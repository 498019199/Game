#include <editor/EditorDialogBoxManager.h>
#include <common/Util.h>

namespace EditorWorker
{
std::unique_ptr<EditorDialogBoxManager> EditorDialogBoxManager::editor_box_mgr_instance_ = nullptr;

EditorDialogBoxManager::EditorDialogBoxManager()
{
    
}

EditorDialogBoxManager::~EditorDialogBoxManager()
{
    
}

EditorDialogBoxManager& EditorDialogBoxManager::Instance()
{
    if (!editor_box_mgr_instance_)
    {
        if (!editor_box_mgr_instance_)
        {
            editor_box_mgr_instance_ = CommonWorker::MakeUniquePtr<EditorDialogBoxManager>();
        }
    }
    return *editor_box_mgr_instance_;
}

void EditorDialogBoxManager::OnRender()
{
    if(message_list_.empty())
    {
        return ;
    }
    const MessageData& msg = message_list_.front();

    ImGuiIO& io = ImGui::GetIO();
    ImGuiStyle& style = ImGui::GetStyle();

    // 窗口居中
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    // 标题(居中)
    style.WindowTitleAlign.x = 0.5f;
    ImGui::Begin(msg.title.c_str(), 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);
    style.WindowTitleAlign.x = 0.0f;

    // 内容
    std::string str = "\n" + msg.content + "\n\n";
    ImGui::Text(str.c_str());

    // 按钮(居中)
    float size = ImGui::CalcTextSize("OK").x + style.FramePadding.x * 2.0f;
    float avail = ImGui::GetContentRegionAvail().x;
    float off = (avail - size) * 0.5f; // 0.5f, 即居中
    if (off > 0.0f)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

    if (ImGui::Button("OK"))
        message_list_.pop_front();

    ImGui::End();
}

void EditorDialogBoxManager::PopMessage(const std::string& title, const std::string& content)
{
    message_list_.push_back({ title, content });
}
    

}
