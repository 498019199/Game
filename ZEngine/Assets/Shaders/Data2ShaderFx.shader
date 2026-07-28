Shader "Data2ShaderFx"
{
    // Dota2 Workshop-style 4 maps:
    //   color RGB + A(translucency), normal, mask1, mask2
    //   mask1: R=detailMask G=diffuse/fresnel B=metalness A=selfIllum
    //   mask2: R=specular G=rim B=tintByBase A=specularExponent (reserved)
    // Extras kept separate: detail, detail2, cubeMap
    Include "util.shader"
    Include "Quaternion.shader"
    Include "Material.shader"
    Include "Mesh.shader"

    Float4x4 worldviewproj semantic="WORLDVIEWPROJECTION"
    Float3 light_dir x=0.35 y=0.85 z=0.45
    Float3 eye_pos_os x=0 y=0 z=0
    Float normal_blend value=0.0
    Float debug_albedo_only value=0.0
    Int debug_mask1_ra value=0

    Float detail_scale value=4.0
    Float detail_strength value=1.0
    Float detail2_scale value=8.0
    Float detail2_strength value=1.0
    Float cubemap_strength value=0.35
    Float selfillum_strength value=1.0

    Int detail_map_enabled value=0
    Int detail2_map_enabled value=0
    Int detail_mask_enabled value=0
    Int cubemap_map_enabled value=0
    Int selfillum_map_enabled value=0
    Int translucency_map_enabled value=0
    Int mask1_map_enabled value=0
    Int mask2_map_enabled value=0

    Texture2D detail_tex
    Texture2D detail2_tex
    Texture2D detail_mask_tex
    Texture2D cubemap_tex
    Texture2D translucency_tex
    Texture2D mask1_tex
    Texture2D mask2_tex

    Sampler linear_sampler
    {
        State filtering = min_mag_mip_linear
        State address_u = wrap
        State address_v = wrap
    }

    Sampler cubemap_sampler
    {
        State filtering = min_mag_mip_linear
        State address_u = clamp
        State address_v = clamp
    }

    SubShader
    {
        Pass
        {
            Name "Data2ShaderFxTech"
            Cull Back

            HLSLPROGRAM
            #pragma vertex Data2ShaderFxVS
            #pragma fragment Data2ShaderFxPS

            float2 ReflectToLatLongUV(float3 r)
            {
                r = normalize(r);
                float u = atan2(r.x, r.z) * (0.5f / 3.14159265f) + 0.5f;
                float v = asin(clamp(r.y, -1.0f, 1.0f)) * (1.0f / 3.14159265f) + 0.5f;
                return float2(u, 1.0f - v);
            }

            void Data2ShaderFxVS(float2 tex0 : TEXCOORD0,
                        float4 pos : POSITION,
                        float4 tangent_quat : TANGENT,
                        out float2 oTex0 : TEXCOORD0,
                        out float3 oTsX : TEXCOORD1,
                        out float3 oTsY : TEXCOORD2,
                        out float3 oTsZ : TEXCOORD3,
                        out float3 oPosOs : TEXCOORD4,
                        out float4 oPos : SV_Position)
            {
                pos = float4(pos.xyz * pos_extent + pos_center, 1);
                tex0 = tex0 * tc_extent + tc_center;
                tangent_quat = tangent_quat * 2 - 1;

                oTex0 = tex0;
                oTsX = transform_quat(float3(1, 0, 0), tangent_quat);
                oTsY = transform_quat(float3(0, 1, 0), tangent_quat) * sign(tangent_quat.w);
                oTsZ = transform_quat(float3(0, 0, 1), tangent_quat);
                oPosOs = pos.xyz;
                oPos = mul(pos, worldviewproj);
            }

            float4 Data2ShaderFxPS(float2 uv : TEXCOORD0,
                        float3 ts_x : TEXCOORD1,
                        float3 ts_y : TEXCOORD2,
                        float3 ts_z : TEXCOORD3,
                        float3 pos_os : TEXCOORD4) : SV_Target
            {
                float4 color_s = albedo_tex.Sample(linear_sampler, uv);
                float3 albedo = color_s.rgb;
                if (dot(albedo, albedo) < 1e-6f)
                {
                    albedo = max(albedo_clr.rgb, float3(0.6f, 0.6f, 0.6f));
                }

                float4 mask1 = float4(0, 0, 0, 0);
                if (mask1_map_enabled)
                {
                    mask1 = mask1_tex.Sample(linear_sampler, uv);
                }

                // detailMask: packed mask1.R, else optional unpacked detail_mask
                float dmask = 1.0f;
                if (mask1_map_enabled)
                {
                    dmask = mask1.r;
                }
                else if (detail_mask_enabled)
                {
                    dmask = detail_mask_tex.Sample(linear_sampler, uv).r;
                }

                if (detail_map_enabled)
                {
                    float3 detail = detail_tex.Sample(linear_sampler, uv * detail_scale).rgb;
                    albedo += detail * dmask * detail_strength;
                }
                if (detail2_map_enabled)
                {
                    float3 detail2 = detail2_tex.Sample(linear_sampler, uv * detail2_scale).rgb;
                    albedo += detail2 * dmask * detail2_strength;
                }

                if (debug_albedo_only > 0.5f)
                {
                    return float4(albedo, 1.0f);
                }
                if (debug_mask1_ra == 1)
                {
                    // Visualize mask1.R (detail mask)
                    return float4(dmask.xxx, 1.0f);
                }

                float3 N = float3(0, 0, 1);
                if (normal_map_enabled)
                {
                    float4 normal_s = normal_tex.Sample(linear_sampler, uv);
                    N = decompress_normal(normal_s);
                    N.xy *= normal_scale;
                    N = normalize(N);
                }

                // metalness: packed mask1.B, else metalness_glossiness_tex.x
                float metalness = metalness_glossiness_factor.x;
                if (mask1_map_enabled)
                {
                    metalness *= mask1.b;
                }
                else if (metalness_glossiness_factor.z > 0.5f)
                {
                    metalness *= get_x_channel(metalness_glossiness_tex.Sample(linear_sampler, uv));
                }

                float3x3 obj_to_ts = float3x3(ts_x, ts_y, ts_z);
                float3 L = normalize(mul(obj_to_ts, normalize(light_dir)));
                float ndotl = saturate(dot(N, L));

                float3 diffuse = albedo * (1.0f - metalness * 0.85f);
                float3 lit = diffuse * (0.40f + 0.60f * ndotl);

                if (cubemap_map_enabled)
                {
                    float3 N_os = normalize(mul(N, transpose(obj_to_ts)));
                    float3 V_os = normalize(eye_pos_os - pos_os);
                    float3 R_os = reflect(-V_os, N_os);
                    float3 env = cubemap_tex.Sample(cubemap_sampler, ReflectToLatLongUV(R_os)).rgb;
                    float fresnel = pow(1.0f - saturate(dot(N, normalize(mul(obj_to_ts, V_os)))), 3.0f);
                    lit = lerp(lit, env, saturate((metalness * 0.65f + fresnel * 0.35f) * cubemap_strength));
                }

                // selfIllum: packed mask1.A, else emissive_tex
                if (selfillum_map_enabled)
                {
                    float self_mask = mask1_map_enabled
                        ? mask1.a
                        : emissive_tex.Sample(linear_sampler, uv).r;
                    lit += self_mask * albedo * selfillum_strength;
                    if (debug_mask1_ra == 2)
                    {
                        // Visualize mask1.A / emissive fallback
                        return float4(self_mask.xxx, 1.0f);
                    }
                }

                // translucency: packed color.A, else optional translucency map
                float alpha = color_s.a;
                if (!mask1_map_enabled && translucency_map_enabled)
                {
                    alpha = saturate(translucency_tex.Sample(linear_sampler, uv).r);
                }

                // mask2 reserved (specular/rim/tint/exponent) — sample so binding stays live
                if (mask2_map_enabled)
                {
                    float4 mask2 = mask2_tex.Sample(linear_sampler, uv);
                    lit += albedo * mask2.g * 0.05f; // soft rim contribution
                }

                float3 n_vis = N * 0.5f + 0.5f;
                float3 clr = lerp(lit, n_vis * albedo, saturate(normal_blend));
                return float4(clr, alpha);
            }
            ENDHLSL
        }
    }
}
