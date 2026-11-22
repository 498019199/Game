#pragma once

#include <render/EffectDesc.h>
#include <span>

namespace RenderWorker
{
// Effect identity and composition live in C++; shader algorithms remain in HLSL.
class ZENGINE_CORE_API CameraFrustumEffect
{
public:
    static EffectDesc Describe();
};

class ZENGINE_CORE_API CameraProxyEffect
{
public:
    static EffectDesc Describe();
};

class ZENGINE_CORE_API Data2ShaderFxEffect
{
public:
    static EffectDesc Describe();
};

class ZENGINE_CORE_API EditorLitV1Effect
{
public:
    static EffectDesc Describe();
};

class ZENGINE_CORE_API GBufferEffect
{
public:
    static EffectDesc Describe();
};

class ZENGINE_CORE_API LightSourceProxyEffect
{
public:
    static EffectDesc Describe();
};

class ZENGINE_CORE_API MLShaderEffect
{
public:
    static EffectDesc Describe();
};

class ZENGINE_CORE_API PredefinedCBuffersEffect
{
public:
    static EffectDesc Describe();
};

class ZENGINE_CORE_API RmlUiEffect
{
public:
    static EffectDesc Describe();
};

class ZENGINE_CORE_API SimpleAlbedoNormalEffect
{
public:
    static EffectDesc Describe();
};

class ZENGINE_CORE_API SkyBoxEffect
{
public:
    static EffectDesc Describe();
};

class ZENGINE_CORE_API SubSurfaceEffect
{
public:
    static EffectDesc Describe();
};

class ZENGINE_CORE_API TerrainEffect
{
public:
    static EffectDesc Describe();
};

// Names are stable IDs used by material data and resource callers.
ZENGINE_CORE_API EffectDesc DescribeBuiltInEffect(std::string_view name);
ZENGINE_CORE_API std::span<std::string_view const> BuiltInEffectNames();
}
