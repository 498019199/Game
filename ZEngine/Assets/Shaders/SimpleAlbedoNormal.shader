Shader "SimpleAlbedoNormal"
{
    // Forward preview: show albedo first, then soft Lambert (no distance falloff).
    Include "util.shader"
    Include "Quaternion.shader"
    Include "Material.shader"
    Include "Mesh.shader"

    Float4x4 worldviewproj semantic="WORLDVIEWPROJECTION"
    Float3 light_dir x=0.35 y=0.85 z=0.45
    Float normal_blend value=0.0
    // 0 = lit preview, 1 = raw albedo (debug)
    Float debug_albedo_only value=0.0

    Sampler linear_sampler
    {
        State filtering = min_mag_mip_linear
        State address_u = wrap
        State address_v = wrap
    }

    SubShader
    {
        Pass
        {
            Name "SimpleAlbedoNormalTech"
            Cull Back

            HLSLPROGRAM
            #pragma vertex SimpleAlbedoNormalVS
            #pragma fragment SimpleAlbedoNormalPS

            void SimpleAlbedoNormalVS(float2 tex0 : TEXCOORD0,
                        float4 pos : POSITION,
                        float4 tangent_quat : TANGENT,
                        out float2 oTex0 : TEXCOORD0,
                        out float3 oTsX : TEXCOORD1,
                        out float3 oTsY : TEXCOORD2,
                        out float3 oTsZ : TEXCOORD3,
                        out float4 oPos : SV_Position)
            {
                pos = float4(pos.xyz * pos_extent + pos_center, 1);
                tex0 = tex0 * tc_extent + tc_center;
                tangent_quat = tangent_quat * 2 - 1;

                oTex0 = tex0;
                oTsX = transform_quat(float3(1, 0, 0), tangent_quat);
                oTsY = transform_quat(float3(0, 1, 0), tangent_quat) * sign(tangent_quat.w);
                oTsZ = transform_quat(float3(0, 0, 1), tangent_quat);
                oPos = mul(pos, worldviewproj);
            }

            float4 SimpleAlbedoNormalPS(float2 uv : TEXCOORD0,
                        float3 ts_x : TEXCOORD1,
                        float3 ts_y : TEXCOORD2,
                        float3 ts_z : TEXCOORD3) : SV_Target
            {
                float3 albedo_s = albedo_tex.Sample(linear_sampler, uv).rgb;
                float4 normal_s = normal_tex.Sample(linear_sampler, uv);

                // Always prefer sampled albedo for this preview path. FBX albedo_clr
                // is often 0; map-enabled flag has also been unreliable across CB clones.
                float3 albedo = albedo_s;
                if (dot(albedo, albedo) < 1e-6f)
                {
                    albedo = max(albedo_clr.rgb, float3(0.6f, 0.6f, 0.6f));
                }

                if (debug_albedo_only > 0.5f)
                {
                    return float4(albedo, 1.0f);
                }

                float3 N = float3(0, 0, 1);
                if (normal_map_enabled)
                {
                    N = decompress_normal(normal_s);
                    N.xy *= normal_scale;
                    N = normalize(N);
                }

                // Directional light in object space → tangent space. No distance
                // attenuation (point-light dist was wiping the image to black when
                // light_pos / mesh spaces disagreed).
                float3x3 obj_to_ts = float3x3(ts_x, ts_y, ts_z);
                float3 L = normalize(mul(obj_to_ts, normalize(light_dir)));
                float ndotl = saturate(dot(N, L));

                float3 lit = albedo * (0.45f + 0.55f * ndotl);
                float3 n_vis = N * 0.5f + 0.5f;
                float3 clr = lerp(lit, n_vis * albedo, saturate(normal_blend));
                return float4(clr, 1.0f);
            }
            ENDHLSL
        }
    }
}
