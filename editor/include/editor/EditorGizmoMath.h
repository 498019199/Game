#pragma once
#include <math/math.h>
#include <imgui.h>
#include <array>
#include <cmath>

namespace EditorWorker::GizmoMath
{
using namespace RenderWorker;
inline bool Editable(float4x4 const& matrix)
{
    for (float v : matrix) if (!std::isfinite(v)) return false;
    float3 x(matrix(0,0),matrix(0,1),matrix(0,2));
    float3 y(matrix(1,0),matrix(1,1),matrix(1,2));
    float3 z(matrix(2,0),matrix(2,1),matrix(2,2));
    float size = MathWorker::length(x)*MathWorker::length(y)*MathWorker::length(z);
    return size > 1e-12f && std::abs(MathWorker::dot(x, MathWorker::cross(y,z))) > size*1e-6f;
}
inline float4x4 LocalFromWorld(float4x4 const& world, float4x4 const& parent)
{
    return world * MathWorker::inverse(parent);
}
// Both the engine's row-vector matrices and ImGuizmo's column-vector matrices
// have translation at offsets 12..14; pass their storage directly, no transpose.
inline std::array<float3,8> FrustumCorners(float4x4 const& inverse_view_projection)
{
    std::array<float3,8> corners;
    for (int i=0;i<8;++i)
        corners[i] = MathWorker::transform_coord(float3((i&1)?1.f:-1.f,(i&2)?1.f:-1.f,(i&4)?1.f:0.f), inverse_view_projection);
    return corners;
}
// Use explicit lens parameters for perspective cameras: inverse projection loses
// the far plane when far/near exceeds float precision (e.g. .001 .. 100000).
inline std::array<float3,8> PerspectiveCorners(float fov, float aspect, float near_plane, float far_plane, float4x4 const& world)
{
    std::array<float3,8> corners;
    for (int i=0;i<8;++i) {
        float z=(i&4)?far_plane:near_plane;
        float y=z*std::tan(fov*.5f), x=y*aspect;
        corners[i]=MathWorker::transform_coord(float3((i&1)?x:-x,(i&2)?y:-y,z),world);
    }
    return corners;
}
// Clip in homogeneous space (LH, depth 0..1), before perspective division.
// This also handles edges crossing the eye and huge far-plane rectangles.
inline bool ProjectLine(float3 const& a, float3 const& b, float4x4 const& vp,
    ImVec2 origin, ImVec2 size, ImVec2& out_a, ImVec2& out_b)
{
    if (size.x <= 0 || size.y <= 0) return false;
    auto p = MathWorker::transform(float4(a.x(),a.y(),a.z(),1), vp);
    auto q = MathWorker::transform(float4(b.x(),b.y(),b.z(),1), vp);
    for (size_t i=0;i<4;++i) if (!std::isfinite(p[i]) || !std::isfinite(q[i])) return false;
    auto distance = [](float4 const& v, int plane) {
        switch (plane) {
        case 0: return v.w()+v.x(); case 1: return v.w()-v.x();
        case 2: return v.w()+v.y(); case 3: return v.w()-v.y();
        case 4: return v.z(); default: return v.w()-v.z();
        }
    };
    for (int plane=0;plane<6;++plane) {
        float x=distance(p,plane), y=distance(q,plane);
        if (x<0 && y<0) return false;
        if ((x<0)!=(y<0)) {
            auto intersection = p+(q-p)*(x/(x-y));
            if (x<0) p=intersection; else q=intersection;
        }
    }
    if (p.w() < 1e-6f || q.w() < 1e-6f) return false;
    auto screen = [&](float4 const& v) { return ImVec2(origin.x+(v.x()/v.w()+1)*size.x*.5f, origin.y+(1-v.y()/v.w())*size.y*.5f); };
    out_a=screen(p); out_b=screen(q); return true;
}
}
