Shader "Terrain"
{
    Include "util.shader"
    Include "PostProcess.shader"
    Include "Mesh.shader"

    Float3 LightDir x=0 y=10 z=3

    Texture2D grass_tex
    Texture2D height_map_tex
    Texture2D normal_map_tex
    Texture2D src_tex
    Texture2D tex_with_alpha

    Sampler point_sampler
    {
        State filtering = min_mag_mip_point
        State address_u = clamp
        State address_v = clamp
    }

    Sampler trilinear_sampler
    {
        State filtering = min_mag_mip_linear
        State address_u = wrap
        State address_v = wrap
        State address_w = wrap
    }

    Sampler normal_map_sampler
    {
        State filtering = min_mag_linear_mip_point
        State address_u = clamp
        State address_v = clamp
    }

    CBuffer per_frame
    {
        Float4x4 mvp
        Float inv_far
    }

    SubShader
    {
        Pass
        {
            Name "Terrain"

            HLSLPROGRAM
            #pragma vertex TerrainVS
            #pragma fragment TerrainPS

            void TerrainVS(float4 pos : POSITION,
                        float2 tex : TEXCOORD0,
                        out float4 oTex : TEXCOORD0,
                        out float oDepth : TEXCOORD1,
                        out float4 oPos : SV_Position)
            {
                pos = float4(pos.xyz * pos_extent + pos_center, 1);

                oTex.xy = float2(tex.x, 1 - tex.y);
                oTex.zw = oTex.xy * 8;

                pos = pos.xzyw;
                pos.y += height_map_tex.SampleLevel(point_sampler, oTex.xy, 0).r;

                oPos = mul(pos, mvp);
                oDepth = oPos.w;
            }

            float4 TerrainPS(float4 tex : TEXCOORD0, float depth : TEXCOORD1) : SV_Target
            {
                float3 normal = normalize(decompress_normal(normal_map_tex.Sample(normal_map_sampler, tex.xy)).xzy * float3(4, 1, 4));
                return float4(grass_tex.Sample(trilinear_sampler, tex.zw).rgb * dot(normalize(LightDir), normal),
                    depth * inv_far);
            }
            ENDHLSL
        }

        Pass
        {
            Name "Blend"
            ZWrite Off

            HLSLPROGRAM
            #pragma vertex PostProcessVS
            #pragma fragment BlendPS

            float4 BlendPS(float2 tex : TEXCOORD0) : SV_Target
            {
                float4 fog = tex_with_alpha.Sample(point_sampler, tex);
                float clr = max(0, dot(LightDir, fog.xyz) / sqrt(dot(LightDir, LightDir) * dot(fog.xyz, fog.xyz)));
                fog.rgb = clr;
                fog.a = saturate(fog.a);
                float4 s = src_tex.Sample(point_sampler, tex);
                return float4(lerp(s.rgb, fog.rgb, fog.a), 1);
            }
            ENDHLSL
        }
    }
}
