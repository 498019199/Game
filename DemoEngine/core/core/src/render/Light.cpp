#include <Render/Light.h>

// LightSource::LightSource(LightType type)
//     :type_(type)
// {
// }

// LightSource::~LightSource() noexcept = default;

// LightSource::LightType LightSource::Type() const
// {
//     return type_;
// }

// const float3& LightSource::Position() const
// {
//     auto mat = this->TransformToWorld();
//     return *reinterpret_cast<float3 const*>(&mat.Row(3)); 
// }

// const float3& LightSource::Direction() const
// {
//     auto mat = this->TransformToWorld();
//     return float3(0, 0, 0);
// }

// quater LightSource::Rotation() const
// {
//     auto mat = this->TransformToWorld();
//     return quater(0, 0, 0, 0);
// }

// const float3& LightSource::Falloff() const
// {
//     return falloff_;
// }

// void LightSource::Falloff(float3 const & fall_off)
// {
    
// }

// float LightSource::Range() const
// {
//     return range_;
// }

// void LightSource::Range(float range)
// {
//     range = 0.f;
// }
