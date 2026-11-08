#include <editor/EditorSceneGizmos.h>
#include <editor/EditorManager.h>
#include <editor/EditorGizmoMath.h>
#include <ImGuizmo.h>
#include <base/Window.h>

namespace EditorWorker
{
using namespace RenderWorker;
namespace
{
void CameraOverlay(EditorManager& editor, ImVec2 origin, ImVec2 size)
{
    auto const camera = editor.Scene().SceneCamera();
    auto const node = editor.Scene().SceneCameraNode();
    if (!camera || !node) return;
    node->UpdateTransforms();
    camera->Dirty();
    auto const vp = editor.ActiveCamera().ViewMatrix()*editor.ActiveCamera().ProjMatrix();
    auto const corners = GizmoMath::PerspectiveCorners(camera->FOV(),camera->Aspect(),camera->NearPlane(),camera->FarPlane(),camera->InverseViewMatrix());
    bool const selected = editor.IsHierarchyItemSelected(node.get());
    auto draw = ImGui::GetWindowDrawList();
    auto line = [&](float3 const& a, float3 const& b, ImU32 color, float width) {
        ImVec2 p,q;
        if (GizmoMath::ProjectLine(a,b,vp,origin,size,p,q)) draw->AddLine(p,q,color,width);
    };
    // A short camera silhouette is always visible; the true near/far planes
    // and all twelve frustum edges appear when this camera is selected.
    auto const inv_view = camera->InverseViewMatrix();
    float const icon_size = (std::max)(.12f, MathWorker::length(camera->EyePos()-editor.ActiveCamera().EyePos())*.025f);
    auto point = [&](float x,float y,float z) { return MathWorker::transform_coord(float3(x,y,z)*icon_size, inv_view); };
    ImU32 const color = selected ? IM_COL32(255,218,98,255) : IM_COL32(210,220,235,190);
    for (int i=0;i<4;++i) {
        int const next = i^((i&1)==((i>>1)&1)?1:2);
        auto a=point((i&1)?1:-1,(i&2)?.65f:-.65f,1.5f);
        auto b=point((next&1)?1:-1,(next&2)?.65f:-.65f,1.5f);
        line(a,b,color,1.5f); line(camera->EyePos(),a,color,1.5f);
    }
    if (selected) {
        for (int i=0;i<8;++i) for (int bit : {1,2,4})
            if (!(i&bit)) line(corners[i],corners[i|bit],color,1.5f);
        // Camera up indicator, like the orientation triangle on Unity cameras.
        line(point(-.5f,.8f,1.5f),point(0,1.3f,1.5f),color,2);
        line(point(0,1.3f,1.5f),point(.5f,.8f,1.5f),color,2);
    }
}
}

void EditorSceneGizmos::Cancel()
{
    if (drag_node_) {
        drag_node_->TransformToParent(original_); drag_node_->UpdateTransforms();
        if (auto camera=drag_node_->FirstComponentOfType<Camera>()) camera->Dirty();
        drag_node_.reset(); suppress_until_release_ = true;
    }
    ImGuizmo::Enable(false);
}

void EditorSceneGizmos::Deactivate(EditorManager& editor)
{
    Cancel();
    Navigate(editor, false);
}

void EditorSceneGizmos::Navigate(EditorManager& editor, bool active)
{
    auto const& io = ImGui::GetIO();
    if (active) {
        if (!navigating_) navigation_.AttachCamera(editor.ActiveCamera());
        navigation_.Scalers(.003f, io.KeyShift ? 75.f : 15.f);
        if (navigating_) navigation_.RotateRel(io.MouseDelta.x, io.MouseDelta.y, 0);
        auto axis = [](ImGuiKey positive, ImGuiKey negative) {
            return float(ImGui::IsKeyDown(positive)) - float(ImGui::IsKeyDown(negative));
        };
        float const dt = (std::min)(io.DeltaTime, .05f);
        navigation_.Move(axis(ImGuiKey_D, ImGuiKey_A)*dt,
            axis(ImGuiKey_E, ImGuiKey_Q)*dt, axis(ImGuiKey_W, ImGuiKey_S)*dt);
    } else if (navigating_) {
        navigation_.DetachCamera();
        // Refresh the Game view controller after editor-only navigation.
        editor.Scene().SetupCameraController(editor.ActiveCamera());
    }
    navigating_ = active;
}

bool EditorSceneGizmos::Draw(EditorManager& editor, ImVec2 origin, ImVec2 size, bool hovered)
{
    auto node = editor.SelectedTransformNode();
    auto const& io = ImGui::GetIO();
    bool const popup = ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopupId|ImGuiPopupFlags_AnyPopupLevel);
    bool const window_active = editor.MainWnd() && editor.MainWnd()->Active();
    bool const focused = window_active && ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
    if (drag_node_ && (node != drag_node_ || popup || !focused ||
        ImGui::IsKeyPressed(ImGuiKey_Escape,false) || operation_ != static_cast<int>(editor.GetTransformType()))) Cancel();
    if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) suppress_until_release_ = false;
    bool const shortcuts = window_active && (focused || hovered) && !io.WantTextInput && !ImGui::IsAnyItemActive() && !popup
        && !io.KeyCtrl && !io.KeySuper && !io.KeyAlt && !ImGui::IsMouseDown(ImGuiMouseButton_Right) && !drag_node_;
    if (shortcuts) {
        if (ImGui::IsKeyPressed(ImGuiKey_T,false)) editor.SetTransformType(ETransformType::TransformType_Position);
        if (ImGui::IsKeyPressed(ImGuiKey_R,false)) editor.SetTransformType(ETransformType::TransformType_Rotation);
        if (ImGui::IsKeyPressed(ImGuiKey_S,false)) editor.SetTransformType(ETransformType::TransformType_Scale);
        if (ImGui::IsKeyPressed(ImGuiKey_F,false)) editor.FrameSelection();
    }
    // Scene navigation reads ImGui directly. Runtime/Rml input stays blocked,
    // so editor shortcuts never also become gameplay actions.
    Navigate(editor, hovered && focused && !popup && !io.WantTextInput && !drag_node_
        && ImGui::IsMouseDown(ImGuiMouseButton_Right));
    auto draw = ImGui::GetWindowDrawList();
    draw->PushClipRect(origin,ImVec2(origin.x+size.x,origin.y+size.y),true);
    CameraOverlay(editor,origin,size);
    // Toolbar is inside the image so the matrix viewport always matches it.
    ImVec2 const cursor=ImGui::GetCursorScreenPos();
    ImGui::SetCursorScreenPos(ImVec2(origin.x+8,origin.y+8));
    ImGui::BeginGroup();
    draw->AddRectFilled(ImVec2(origin.x+4,origin.y+4),ImVec2(origin.x+(std::min)(size.x-4,480.f),origin.y+(node==editor.Scene().SceneCameraNode() && node ? 42.f:25.f)),IM_COL32(20,24,30,210),3.f);
    ImGui::BeginDisabled(drag_node_ != nullptr);
    if (ImGui::SmallButton(local_ ? "Local" : "World")) local_ = !local_;
    ImGui::SameLine(); if (ImGui::SmallButton("Frame (F)")) editor.FrameSelection();
    ImGui::EndDisabled();
    ImGui::SameLine(); ImGui::TextUnformatted("T Move | R Rotate | S Scale | RMB + WASD: fly");
    if (node == editor.Scene().SceneCameraNode() && node) {
        auto const& c=*editor.Scene().SceneCamera();
        ImGui::Text("Camera: FOV %.1f  Aspect %.3f  Near %.3f  Far %.1f",MathWorker::Rad2Deg(c.FOV()),c.Aspect(),c.NearPlane(),c.FarPlane());
    }
    ImGui::EndGroup();
    bool const controls_hovered = ImGui::IsItemHovered();
    ImGui::SetCursorScreenPos(cursor);
    ImGui::Dummy(ImVec2(0,0)); // Finalize the restored layout cursor for ImGui 1.92.

    bool captured = controls_hovered;
    if (node) {
        node->UpdateTransforms();
        auto matrix = node->TransformToWorld();
        auto const before = node->TransformToParent();
        auto const parent = node->Parent() ? node->Parent()->TransformToWorld() : float4x4::Identity();
        auto const type = editor.GetTransformType();
        bool const camera_scale = node == editor.Scene().SceneCameraNode() && type == ETransformType::TransformType_Scale;
        if (GizmoMath::Editable(matrix) && GizmoMath::Editable(parent) && !camera_scale) {
            ImGuizmo::SetDrawlist(draw);
            ImGuizmo::SetRect(origin.x,origin.y,size.x,size.y);
            ImGuizmo::SetOrthographic(editor.ActiveCamera().FOV()==0.f);
            ImGuizmo::AllowAxisFlip(false);
            // A captured drag may pass over the toolbar without committing early.
            bool enabled = window_active && !suppress_until_release_ && !popup && (!controls_hovered || drag_node_)
                && !io.WantTextInput && !ImGui::IsMouseDown(ImGuiMouseButton_Right);
            ImGuizmo::Enable(enabled);
            auto operation = type==ETransformType::TransformType_Position ? ImGuizmo::TRANSLATE
                : type==ETransformType::TransformType_Rotation ? ImGuizmo::ROTATE : ImGuizmo::SCALE;
            // Scaling is along the object's local axes to preserve its S*R*T basis.
            auto mode=(local_ || operation==ImGuizmo::SCALE) ? ImGuizmo::LOCAL : ImGuizmo::WORLD;
            auto const& camera=editor.ActiveCamera();
            bool changed = ImGuizmo::Manipulate(camera.ViewMatrix().data(),camera.ProjMatrix().data(),operation,mode,matrix.data());
            bool const using_gizmo=ImGuizmo::IsUsing();
            if (using_gizmo && !drag_node_) { drag_node_=node; original_=before; operation_=static_cast<int>(type); }
            if (changed && GizmoMath::Editable(matrix)) {
                node->TransformToParent(GizmoMath::LocalFromWorld(matrix,parent)); node->UpdateTransforms();
                if (auto scene_camera=node->FirstComponentOfType<Camera>()) scene_camera->Dirty();
            }
            captured |= ImGuizmo::IsOver() || using_gizmo || suppress_until_release_;
            if (!using_gizmo && drag_node_) drag_node_.reset();
        } else {
            Cancel();
            draw->AddText(ImVec2(origin.x+8,origin.y+55),IM_COL32(255,210,100,255),
                camera_scale ? "Camera range: edit FOV / Near / Far in Inspector." : "Cannot manipulate a singular transform.");
        }
    } else Cancel();
    draw->PopClipRect();
    return captured;
}
}
