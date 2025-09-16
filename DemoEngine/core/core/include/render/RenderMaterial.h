#pragma once

#include <render/RenderEffect.h>
#include <render/RenderView.h>

namespace RenderWorker
{
class RenderMaterial;
using RenderMaterialPtr = std::shared_ptr<RenderMaterial>;

class ZENGINE_CORE_API RenderMaterial final
{
    ZENGINE_NONCOPYABLE(RenderMaterial);

public:
    enum TextureSlot
    {
        TS_Albedo,
        TS_MetalnessGlossiness,
        TS_Emissive,
        TS_Normal,
        TS_Height,
        TS_Occlusion,

        TS_NumTextureSlots
    };

    enum class SurfaceDetailMode : uint32_t
    {
        ParallaxMapping = 0,
        ParallaxOcclusionMapping,
        FlatTessellation,
        SmoothTessellation
    };

public:
    //RenderMaterial();
};
}