#include <base/ZEngine.h>
#include <editor/EditorGizmoMath.h>
#include <ImGuizmo.h>
#include <gtest/gtest.h>

using namespace RenderWorker;
using namespace EditorWorker;
namespace
{
void Near(float4x4 const& a,float4x4 const& b) { for(size_t i=0;i<16;++i) EXPECT_NEAR(a[i],b[i],2e-4f); }
TEST(EditorGizmo, WorldEditsRoundTripThroughRotatedScaledParent)
{
    auto parent=MathWorker::scaling(2.f,3.f,4.f)*MathWorker::rotation_y(.7f)*MathWorker::translation(5.f,6.f,7.f);
    auto local=MathWorker::rotation_x(.3f)*MathWorker::translation(1.f,2.f,3.f);
    auto world=local*parent;
    Near(GizmoMath::LocalFromWorld(world,parent),local);
    world(3,0)+=4.f;
    auto edited=GizmoMath::LocalFromWorld(world,parent);
    Near(edited*parent,world);
    for(size_t row=0;row<3;++row) for(size_t col=0;col<3;++col) EXPECT_NEAR(local(row,col),edited(row,col),2e-4f);
}
TEST(EditorGizmo, SingularAndNonfiniteTransformsAreNotEditable)
{
    EXPECT_TRUE(GizmoMath::Editable(MathWorker::scaling(-2.f,3.f,4.f)));
    EXPECT_FALSE(GizmoMath::Editable(MathWorker::scaling(0.f,1.f,1.f)));
    auto value=float4x4::Identity();value(1,0)=1;value(1,1)=0;
    EXPECT_FALSE(GizmoMath::Editable(value));
    value=float4x4::Identity();value(3,0)=std::numeric_limits<float>::infinity();
    EXPECT_FALSE(GizmoMath::Editable(value));
}
TEST(EditorGizmo, CameraFrustumMatchesFovAspectNearAndFar)
{
    auto projection=MathWorker::perspective_fov_lh(PI/2,2.f,1.f,10.f);
    auto world=MathWorker::rotation_y(.4f)*MathWorker::translation(2.f,3.f,4.f);
    auto corners=GizmoMath::FrustumCorners(MathWorker::inverse(projection)*world);
    for(int i=0;i<8;++i) {
        auto p=MathWorker::transform_coord(corners[i],MathWorker::inverse(world));
        float z=(i&4)?10:1;
        EXPECT_NEAR(z,p.z(),1e-4f);
        EXPECT_NEAR((i&1)?z*2:-z*2,p.x(),1e-4f);
        EXPECT_NEAR((i&2)?z:-z,p.y(),1e-4f);
    }
}
TEST(EditorGizmo, OrthographicFrustumHasParallelEdges)
{
    auto corners=GizmoMath::FrustumCorners(MathWorker::inverse(MathWorker::OrthoLH(8.f,4.f,1.f,20.f)));
    for(int i=0;i<4;++i) { EXPECT_NEAR(corners[i].x(),corners[i+4].x(),1e-4f); EXPECT_NEAR(corners[i].y(),corners[i+4].y(),1e-4f); }
}
TEST(EditorGizmo, ClipsCameraEdgesCrossingEyeAndViewport)
{
    auto projection=MathWorker::perspective_fov_lh(PI/2,1.f,1.f,10.f);
    ImVec2 a,b;
    ASSERT_TRUE(GizmoMath::ProjectLine(float3(0,0,-2),float3(0,0,5),projection,{20,30},{400,200},a,b));
    EXPECT_NEAR(220,a.x,1e-4f);EXPECT_NEAR(130,a.y,1e-4f);
    ASSERT_TRUE(GizmoMath::ProjectLine(float3(-100,0,5),float3(100,0,5),projection,{20,30},{400,200},a,b));
    EXPECT_NEAR(20,a.x,1e-3f);EXPECT_NEAR(420,b.x,1e-3f);
    EXPECT_FALSE(GizmoMath::ProjectLine(float3(0,0,-5),float3(1,0,-1),projection,{20,30},{400,200},a,b));
    EXPECT_FALSE(GizmoMath::ProjectLine(float3(0,0,20),float3(1,0,30),projection,{20,30},{400,200},a,b));
}
TEST(EditorGizmo, ImGuizmoPickingUsesEngineMatrixLayoutAndViewportOffset)
{
    auto view=MathWorker::look_at_lh(float3(3,2,-5),float3(3,2,0),float3(0,1,0));
    auto projection=MathWorker::perspective_fov_lh(PI/2,1.f,.1f,100.f);
    float3 origin,direction;
    ImGuizmo::ComputeMouseRay(view.data(),projection.data(),{450,350},{200,100},{500,500},origin.data(),direction.data());
    EXPECT_NEAR(3.f,origin.x(),1e-4f);EXPECT_NEAR(2.f,origin.y(),1e-4f);
    EXPECT_NEAR(0.f,direction.x(),1e-4f);EXPECT_NEAR(0.f,direction.y(),1e-4f);EXPECT_NEAR(1.f,direction.z(),1e-4f);
    ImGuizmo::ComputeMouseRay(view.data(),projection.data(),{700,100},{200,100},{500,500},origin.data(),direction.data());
    EXPECT_GT(direction.x(),0);EXPECT_GT(direction.y(),0);EXPECT_GT(direction.z(),0);
}
}

TEST(EditorGizmo, TranslationDragChangesOnlyChosenAxis)
{
    ImGui::CreateContext();
    auto& io=ImGui::GetIO(); io.IniFilename=nullptr; io.DisplaySize={600,600}; io.DeltaTime=1.f/60; io.ConfigInputTrickleEventQueue=false;
    unsigned char* pixels; int width,height; io.Fonts->GetTexDataAsRGBA32(&pixels,&width,&height);
    auto view=MathWorker::look_at_lh(float3(0,0,-5),float3(0,0,0),float3(0,1,0));
    auto proj=MathWorker::perspective_fov_lh(PI/2,1.f,.1f,100.f);
    auto matrix=float4x4::Identity();
    auto frame=[&](float x,float y,bool down) {
        io.AddMousePosEvent(x,y);io.AddMouseButtonEvent(0,down);
        ImGui::NewFrame();ImGuizmo::BeginFrame();
        ImGui::SetNextWindowPos({0,0});ImGui::SetNextWindowSize({600,600});
        ImGui::Begin("GizmoTest",nullptr,ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove);
        ImGuizmo::SetDrawlist();ImGuizmo::SetRect(0,0,600,600);ImGuizmo::SetOrthographic(false);ImGuizmo::Enable(true);ImGuizmo::AllowAxisFlip(false);
        ImGuizmo::Manipulate(view.data(),proj.data(),ImGuizmo::TRANSLATE,ImGuizmo::WORLD,matrix.data());
        ImGui::End();ImGui::Render();
    };
    frame(330,300,false);frame(330,300,false);frame(330,300,true);
    EXPECT_TRUE(ImGuizmo::IsUsing());
    frame(390,300,true);frame(390,300,false);
    EXPECT_FALSE(ImGuizmo::IsUsing());
    EXPECT_NEAR(1.f,matrix(3,0),.02f);EXPECT_NEAR(0.f,matrix(3,1),1e-4f);EXPECT_NEAR(0.f,matrix(3,2),1e-4f);
    ImGuizmo::Enable(false);ImGui::DestroyContext();
}

TEST(EditorGizmo, HugeFarNearRatioKeepsFiniteCameraCorners)
{
    auto corners=GizmoMath::PerspectiveCorners(PI/2,2.f,.001f,100000.f,float4x4::Identity());
    for (auto const& corner:corners) for(size_t i=0;i<3;++i) EXPECT_TRUE(std::isfinite(corner[i]));
    EXPECT_FLOAT_EQ(100000.f,corners[7].z());
    EXPECT_NEAR(200000.f,corners[7].x(),1.f);
    EXPECT_NEAR(.001f,corners[0].z(),1e-7f);
}
namespace
{
class GizmoInput : public testing::Test
{
protected:
    float4x4 matrix=float4x4::Identity();
    float4x4 view=MathWorker::look_at_lh(float3(0,0,-5),float3(0,0,0),float3(0,1,0));
    float4x4 proj=MathWorker::perspective_fov_lh(PI/2,1.f,.1f,100.f);
    ImGuizmo::OPERATION operation=ImGuizmo::ROTATE_Z;
    bool enabled=true;
    void SetUp() override {
        ImGui::CreateContext(); auto& io=ImGui::GetIO();io.IniFilename=nullptr;io.DisplaySize={600,600};io.DeltaTime=1.f/60;io.ConfigInputTrickleEventQueue=false;
        unsigned char* pixels;int w,h;io.Fonts->GetTexDataAsRGBA32(&pixels,&w,&h);
    }
    void TearDown() override { ImGuizmo::Enable(false);ImGui::DestroyContext(); }
    void Frame(float x,float y,bool down) {
        auto& io=ImGui::GetIO();io.AddMousePosEvent(x,y);io.AddMouseButtonEvent(0,down);
        ImGui::NewFrame();ImGuizmo::BeginFrame();
        ImGui::SetNextWindowPos({0,0});ImGui::SetNextWindowSize({600,600});
        ImGui::Begin("GizmoInput",nullptr,ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove);
        ImGuizmo::SetDrawlist();ImGuizmo::SetRect(0,0,600,600);ImGuizmo::SetOrthographic(false);ImGuizmo::Enable(enabled);ImGuizmo::AllowAxisFlip(false);
        ImGuizmo::Manipulate(view.data(),proj.data(),operation,ImGuizmo::LOCAL,matrix.data());
        ImGui::End();ImGui::Render();
    }
};
TEST_F(GizmoInput, RotationRingMakesQuarterTurnWithoutTranslationOrScale)
{
    Frame(300,300,false);Frame(300,300,false);
    float radius=0;
    // Find a point on the public widget's rendered/pickable ring, then drag
    // exactly one quarter-circle. No dependency on the library's handle size.
    for(int r=20;r<=100 && radius==0;++r) { Frame(300.f+r,300,false);if(ImGuizmo::IsOver(operation)) radius=static_cast<float>(r); }
    ASSERT_GT(radius,0);
    Frame(300+radius,300,true);ASSERT_TRUE(ImGuizmo::IsUsing());
    Frame(300,300-radius,true);Frame(300,300-radius,false);
    EXPECT_NEAR(0,matrix(0,0),.01f);EXPECT_NEAR(1,std::abs(matrix(0,1)),.01f);
    EXPECT_NEAR(1,std::abs(matrix(1,0)),.01f);EXPECT_NEAR(0,matrix(1,1),.01f);
    for(size_t i=0;i<3;++i) EXPECT_NEAR(0,matrix(3,i),1e-4f);
    EXPECT_NEAR(1,matrix(2,2),1e-4f);
}
TEST_F(GizmoInput, UniformScaleDragPreservesCenterAndAxisRatios)
{
    operation=ImGuizmo::SCALE;matrix=MathWorker::scaling(2.f,3.f,4.f);
    Frame(300,300,false);Frame(300,300,false);Frame(300,300,true);
    ASSERT_TRUE(ImGuizmo::IsUsing());
    Frame(360,300,true);Frame(360,300,false);
    float factor=matrix(0,0)/2;EXPECT_GT(factor,1.f);
    EXPECT_NEAR(factor,matrix(1,1)/3,1e-4f);EXPECT_NEAR(factor,matrix(2,2)/4,1e-4f);
    for(size_t i=0;i<3;++i) EXPECT_NEAR(0,matrix(3,i),1e-4f);
}
TEST_F(GizmoInput, CancelledDragCannotMutateRestoredTransform)
{
    operation=ImGuizmo::TRANSLATE;
    Frame(330,300,false);Frame(330,300,false);Frame(330,300,true);ASSERT_TRUE(ImGuizmo::IsUsing());
    Frame(390,300,true);EXPECT_GT(matrix(3,0),.5f);
    // Editor cancellation restores its snapshot and disables capture until release.
    matrix=float4x4::Identity();enabled=false;
    Frame(420,300,true);Frame(420,300,false);
    EXPECT_FALSE(ImGuizmo::IsUsing());Near(matrix,float4x4::Identity());
}
}
