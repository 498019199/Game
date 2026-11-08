Shader "Particle"
{
    Include "util.shader"
    Float4x4 particle_view_projection
    Float particle_style
    Float particle_glow
    Float4 particle_refraction
    Texture2D particle_scene_color
    Sampler particle_scene_sampler
    {
        State filtering = min_mag_mip_linear
        State address_u = clamp
        State address_v = clamp
    }

    HLSLPROGRAM
    void ParticleVS(float3 pos : POSITION, float4 color : COLOR, float2 uv : TEXCOORD0, float age : TEXCOORD1,
        out float4 oColor : COLOR, out float2 oUv : TEXCOORD0, out float oAge : TEXCOORD1,
        out float4 oPos : SV_Position)
    {
        oPos = mul(float4(pos, 1), particle_view_projection);
        oColor = color;
        oUv = uv;
        oAge = age;
    }

    float4 ParticlePS(float4 color : COLOR, float2 uv : TEXCOORD0) : SV_Target
    {
        float2 p = uv * 2 - 1;
        float radiusSquared = dot(p, p);
        float edge = saturate(1 - radiusSquared);
        // Soft procedural sprites need no external textures. Subtle mottling
        // breaks up smoke; additive sprites retain a brighter, smooth core.
        float smoke = edge * edge * (0.82f + 0.18f * sin(p.x * 9 + sin(p.y * 7)));
        float glow = edge * edge * edge;
        float alpha = saturate(color.a) * lerp(smoke, glow, particle_style);
        // The wider, additive halo is independent of opacity, preserving the
        // soft silhouette while allowing a bright core and luminous fringe.
        float halo = saturate(color.a) * edge * particle_glow;
        return float4(color.rgb * (alpha + halo), alpha);
    }

    float4 ParticleDistortionPS(float4 color : COLOR, float2 uv : TEXCOORD0,
        float age : TEXCOORD1, float4 screenPos : SV_Position) : SV_Target
    {
        float2 p = uv * 2 - 1;
        float edge = saturate(1 - dot(p, p));
        float mask = saturate(color.a) * edge * edge;
        // Sample an independent snapshot of the scene, in render-target pixel
        // coordinates. Animated gradients mimic rising warm air, and fade to
        // zero displacement at the sprite boundary to prevent seams.
        float2 flow = float2(sin(p.y * 9 - age * 5 + sin(p.x * 5)),
            cos(p.x * 7 + age * 4 + sin(p.y * 6)));
        float2 invSize = particle_refraction.xy;
        float2 sceneUv = (screenPos.xy + flow * particle_refraction.z * edge) * invSize;
        sceneUv = clamp(sceneUv, invSize * 0.5f, 1 - invSize * 0.5f);
        float3 background = particle_scene_color.Sample(particle_scene_sampler, sceneUv).rgb;
        return float4(background * mask, mask);
    }
    ENDHLSL

    SubShader
    {
        Pass
        {
            Name "ParticleAlphaTech"
            Cull Off
            ZWrite Off
            ZTest LEqual
            Blend One OneMinusSrcAlpha
            HLSLPROGRAM
            #pragma vertex ParticleVS
            #pragma fragment ParticlePS
            ENDHLSL
        }
        Pass
        {
            Name "ParticleDistortionTech"
            Cull Off
            ZWrite Off
            ZTest LEqual
            Blend One OneMinusSrcAlpha
            HLSLPROGRAM
            #pragma vertex ParticleVS
            #pragma fragment ParticleDistortionPS
            ENDHLSL
        }
        Pass
        {
            Name "ParticleAdditiveTech"
            Cull Off
            ZWrite Off
            ZTest LEqual
            Blend One One
            HLSLPROGRAM
            #pragma vertex ParticleVS
            #pragma fragment ParticlePS
            ENDHLSL
        }
    }
}
