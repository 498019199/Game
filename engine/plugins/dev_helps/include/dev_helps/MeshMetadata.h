#pragma once

#include <base/ZEngine.h>
#include <base/DevHelper.h>
#include <string>
#include <string_view>
#include <vector>

namespace RenderWorker
{
class ZENGINE_CORE_API MeshMetadata final
{
public:
    MeshMetadata();
    explicit MeshMetadata(std::string_view name);

    void Load(std::string_view name);
    void Save(std::string const & name) const;

    bool AutoCenter() const
    {
        return auto_center_;
    }
    void AutoCenter(bool auto_center)
    {
        auto_center_ = auto_center;
    }

    const float3& Pivot() const
    {
        return pivot_;
    }
    void Pivot(const float3& pivot)
    {
        pivot_ = pivot;
    }
    const float3& Translation() const
    {
        return translation_;
    }
    void Translation(const float3& trans)
    {
        translation_ = trans;
    }
    quater const & Rotation() const
    {
        return rotation_;
    }
    void Rotation(const quater& rot)
    {
        rotation_ = rot;
    }
    const float3& Scale() const
    {
        return scale_;
    }
    void Scale(const float3& scale)
    {
        scale_ = scale;
    }

    uint8_t AxisMapping(uint32_t axis) const
    {
        return axis_mapping_[axis];
    }
    void AxisMapping(uint32_t axis, uint8_t mapping)
    {
        axis_mapping_[axis] = mapping;
    }

    bool FlipWindingOrder() const
    {
        return flip_winding_order_;
    }
    void FlipWindingOrder(bool flip_winding_order)
    {
        flip_winding_order_ = flip_winding_order;
    }

    uint32_t NumLods() const;
    void NumLods(uint32_t lods);
    std::string_view LodFileName(uint32_t lod) const;
    void LodFileName(uint32_t lod, std::string_view lod_name);

    uint32_t NumMaterials() const;
    void NumMaterials(uint32_t materials);
    std::string_view MaterialFileName(uint32_t mtl_index) const;
    void MaterialFileName(uint32_t mtl_index, std::string_view mtlml_name);

    const float4x4& Transform() const
    {
        return transform_;
    }
    const float4x4& TransformIT() const
    {
        return transform_it_;
    }

    void UpdateTransforms();

private:
    bool auto_center_ = false;

    float3 pivot_ = float3::Zero();
    float3 translation_ = float3::Zero();
    quater rotation_ = quater::Identity();
    float3 scale_ = float3(1, 1, 1);
    uint8_t axis_mapping_[3] = { 0, 1, 2 };
    bool flip_winding_order_ = false;
    std::vector<std::string> lod_file_names_;
    std::vector<std::string> material_file_names_;

    float4x4 transform_ = float4x4::Identity();
    float4x4 transform_it_ = float4x4::Identity();
};
}