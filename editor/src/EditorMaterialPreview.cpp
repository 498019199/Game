#include <base/ZEngine.h>
#include <editor/EditorMaterialPreview.h>
#include <game/Model.h>
#include <render/RenderEngine.h>
#include <render/RenderFactory.h>
#include <render/FrameBuffer.h>
#include <render/RenderLayout.h>
#include <array>
#include <cmath>
#include <vector>

namespace EditorWorker
{
using namespace RenderWorker;
namespace
{
constexpr float pi = 3.14159265359f;
struct Vertex { float4 position; float2 uv; float4 tangent; };
Vertex MakeVertex(float3 const& p, float2 const& uv, float3 const& tangent, float3 const& normal)
{
    auto q = MathWorker::to_quaternion(tangent, MathWorker::cross(normal, tangent), normal, 16);
    return {float4(p.x(), p.y(), p.z(), 1), uv, float4(q.x() * .5f + .5f, q.y() * .5f + .5f, q.z() * .5f + .5f, q.w() * .5f + .5f)};
}
std::vector<Vertex> Geometry(int shape)
{
    std::vector<Vertex> vertices;
    auto quad = [&](Vertex a, Vertex b, Vertex c, Vertex d) {
        vertices.insert(vertices.end(), {a,b,c,c,b,d});
    };
    if (shape == 0) {
        auto point = [](int y, int x) {
            float theta = pi * static_cast<float>(y) / 32, phi = 2 * pi * static_cast<float>(x) / 48;
            float3 normal(std::sin(theta) * std::cos(phi), std::cos(theta), std::sin(theta) * std::sin(phi));
            return MakeVertex(normal, float2(static_cast<float>(x)/48, static_cast<float>(y)/32), float3(-std::sin(phi),0,std::cos(phi)), normal);
        };
        for (int y = 0; y < 32; ++y) for (int x = 0; x < 48; ++x)
            quad(point(y,x), point(y,x+1), point(y+1,x), point(y+1,x+1));
    } else {
        auto face = [&](float3 normal, float3 tangent) {
            auto bitangent = MathWorker::cross(normal, tangent);
            float3 center = shape == 1 ? normal * .75f : float3(0,0,0);
            float extent = shape == 1 ? .75f : 1;
            quad(MakeVertex(center - tangent*extent - bitangent*extent, float2(0,1), tangent, normal),
                 MakeVertex(center + tangent*extent - bitangent*extent, float2(1,1), tangent, normal),
                 MakeVertex(center - tangent*extent + bitangent*extent, float2(0,0), tangent, normal),
                 MakeVertex(center + tangent*extent + bitangent*extent, float2(1,0), tangent, normal));
        };
        face(float3(0,0,1), float3(1,0,0));
        if (shape == 1) {
            face(float3(0,0,-1), float3(-1,0,0)); face(float3(1,0,0), float3(0,0,-1));
            face(float3(-1,0,0), float3(0,0,1)); face(float3(0,1,0), float3(1,0,0)); face(float3(0,-1,0), float3(1,0,0));
        }
    }
    return vertices;
}
class PreviewMesh final : public DetailedMesh
{
public:
    PreviewMesh() : DetailedMesh(L"MaterialPreview") {}
    void Draw(RenderLayout const& layout, float yaw, float distance)
    {
        OnRenderBegin();
        auto world = MathWorker::rotation_y(yaw);
        float3 eye(0, .25f, distance);
        auto view = MathWorker::look_at_lh(eye, float3(0,0,0), float3(0,1,0));
        auto projection = MathWorker::perspective_fov_lh(pi / 4, 1.0f, .1f, 100.0f);
        *effect_->ParameterByName("worldviewproj") = world * view * projection;
        *effect_->ParameterByName("pos_center") = float3(0,0,0);
        *effect_->ParameterByName("pos_extent") = float3(1,1,1);
        *effect_->ParameterByName("tc_center") = float2(0,0);
        *effect_->ParameterByName("tc_extent") = float2(1,1);
        if (auto p = effect_->ParameterByName("eye_pos_os")) *p = MathWorker::transform_coord(eye, MathWorker::inverse(world));
        Context::Instance().RenderFactoryInstance().RenderEngineInstance().Render(*effect_, *technique_, layout);
    }
};
}
struct EditorMaterialPreview::Impl
{
    FrameBufferPtr framebuffer;
    ShaderResourceViewPtr image;
    std::array<RenderLayoutPtr,3> shapes;
    PreviewMesh mesh;
    Impl()
    {
        auto& rf = Context::Instance().RenderFactoryInstance();
        auto target = rf.MakeTexture2D(512,512,1,1,EF_ABGR8,1,0,EAH_GPU_Read | EAH_GPU_Write);
        framebuffer = rf.MakeFrameBuffer();
        framebuffer->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(target,0,1,0));
        framebuffer->Attach(rf.Make2DDsv(512,512,EF_D16,1,0));
        framebuffer->Viewport()->Camera(rf.RenderEngineInstance().CurFrameBuffer()->Viewport()->Camera());
        image = rf.MakeTextureSrv(target);
        VertexElement format[] = {{VEU_Position,0,EF_ABGR32F},{VEU_TextureCoord,0,EF_GR32F},{VEU_Tangent,0,EF_ABGR32F}};
        for (int i=0; i<3; ++i) {
            auto vertices = Geometry(i);
            auto layout = rf.MakeRenderLayout(); layout->TopologyType(RenderLayout::TT_TriangleList);
            layout->NumVertices(static_cast<uint32_t>(vertices.size()));
            layout->BindVertexStream(rf.MakeVertexBuffer(BU_Static,EAH_GPU_Read | EAH_Immutable,
                static_cast<uint32_t>(vertices.size()*sizeof(Vertex)),vertices.data()),format);
            shapes[i] = std::move(layout);
        }
    }
};
EditorMaterialPreview::EditorMaterialPreview() : impl_(std::make_unique<Impl>()) {}
EditorMaterialPreview::~EditorMaterialPreview() = default;
void EditorMaterialPreview::Render(RenderMaterialPtr const& material, int shape, float yaw, float distance)
{
    auto& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
    auto previous = re.CurFrameBuffer();
    try {
        re.BindFrameBuffer(impl_->framebuffer);
        impl_->framebuffer->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth,Color(.09f,.09f,.11f,1),1,0);
        impl_->mesh.Material(material); impl_->mesh.Draw(*impl_->shapes.at(static_cast<size_t>(shape)),yaw,distance);
        re.BindFrameBuffer(previous);
    } catch (...) { re.BindFrameBuffer(previous); throw; }
}
ShaderResourceViewPtr const& EditorMaterialPreview::Image() const { return impl_->image; }
}
