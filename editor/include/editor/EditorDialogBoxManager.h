#pragma once
#include <string>
#include <deque>
#include <editor/EditorPanel.h>

namespace EditorWorker
{
struct MessageData
{
    std::string title;
    std::string content;
};

class EditorDialogBoxManager
{
public:
    EditorDialogBoxManager();
    ~EditorDialogBoxManager();

    static EditorDialogBoxManager& Instance();
    void OnRender();
    void PopMessage(const std::string& title, const std::string& content);

private:
    static std::unique_ptr<EditorDialogBoxManager> editor_box_mgr_instance_;
    std::deque<MessageData> message_list_;
};

}


#define POP_MESSAGE(title, content) EditorWorker::EditorDialogBoxManager::Instance().PopMessage(title, content)
