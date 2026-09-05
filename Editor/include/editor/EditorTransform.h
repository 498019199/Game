#pragma once

#include <math/math.h>
#include <algorithm>
#include <cmath>

namespace EditorWorker
{
// The engine uses row vectors: S * R * T. Rotation is Rz * Rx * Ry.
inline RenderWorker::float3 InspectorEulerDegrees(RenderWorker::float4x4 const& rotation)
{
    using namespace RenderWorker;
    float const pitch = std::asin(std::clamp(-rotation(2, 1), -1.0f, 1.0f));
    float yaw;
    float roll;
    if (std::abs(std::cos(pitch)) > 1e-5f)
    {
        yaw = std::atan2(rotation(2, 0), rotation(2, 2));
        roll = std::atan2(rotation(0, 1), rotation(1, 1));
    }
    else
    {
        yaw = std::atan2(-rotation(0, 2), rotation(0, 0));
        roll = 0.0f;
    }
    return float3(MathWorker::Rad2Deg(pitch), MathWorker::Rad2Deg(yaw), MathWorker::Rad2Deg(roll));
}

inline RenderWorker::float4x4 InspectorTransform(
    RenderWorker::float4x4 const& original, RenderWorker::float3 const& position,
    RenderWorker::float3 const& rotation_degrees, RenderWorker::float3 const& scale,
    RenderWorker::float3 const& original_scale, bool rotation_changed, bool scale_changed)
{
    using namespace RenderWorker;
    auto result = original;
    if (rotation_changed)
    {
        result = MathWorker::scaling(scale) * MathWorker::rotation_matrix_yaw_pitch_roll(
            MathWorker::Deg2Rad(rotation_degrees.y()), MathWorker::Deg2Rad(rotation_degrees.x()),
            MathWorker::Deg2Rad(rotation_degrees.z()));
    }
    else if (scale_changed)
    {
        // Preserve the existing basis exactly rather than round-tripping Euler angles.
        for (size_t row = 0; row < 3; ++row)
            for (size_t col = 0; col < 3; ++col)
                result(row, col) *= scale[row] / original_scale[row];
    }
    result(3, 0) = position.x();
    result(3, 1) = position.y();
    result(3, 2) = position.z();
    return result;
}
}
