#pragma once
#include <world/SceneNode.h>
#include <world/CameraController.h>
#include <imgui.h>

namespace EditorWorker
{
class EditorManager;
class EditorSceneGizmos
{
public:
    // Returns true when the viewport input is captured by a handle.
    bool Draw(EditorManager& editor, ImVec2 origin, ImVec2 size, bool hovered);
    void Cancel();
    void Deactivate(EditorManager& editor);
private:
    void Navigate(EditorManager& editor, bool active);
    RenderWorker::FirstPersonController navigation_ {false};
    bool navigating_ = false;
    RenderWorker::SceneNodePtr drag_node_;
    RenderWorker::float4x4 original_ = RenderWorker::float4x4::Identity();
    bool suppress_until_release_ = false;
    bool local_ = false;
    int operation_ = -1;
};
}
