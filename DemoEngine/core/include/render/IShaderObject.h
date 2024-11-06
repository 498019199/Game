#pragma once 
#include <common/common.h>

namespace CoreWorker
{
enum class ShaderStage : uint32_t
{
    Vertex,
    Pixel,
    Geometry,
    Compute,
    Hull,
    Domain,

    NumStages,
};

class IShaderObject
{
public:
    IShaderObject() = default;
    ~IShaderObject() = default;    
};
}