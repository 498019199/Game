Shader "EditorLitV1"
{
    // Small opaque forward material, not a full GGX/IBL pipeline.
    Include "util.shader"
    Include "Quaternion.shader"
    Include "Material.shader"
    Include "Mesh.shader"
    Float4x4 worldviewproj semantic="WORLDVIEWPROJECTION"
    Float3 eye_pos_os
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
            Name "EditorLitV1Tech"
            Cull Back
            HLSLPROGRAM
            #pragma vertex EditorLitVS
            #pragma fragment EditorLitPS
            void EditorLitVS(float2 uv : TEXCOORD0, float4 pos : POSITION, float4 tangent_quat : TANGENT,
                out float2 oUv : TEXCOORD0, out float3 oX : TEXCOORD1, out float3 oY : TEXCOORD2,
                out float3 oZ : TEXCOORD3, out float3 oView : TEXCOORD4, out float4 oPos : SV_Position)
            {
                pos = float4(pos.xyz * pos_extent + pos_center, 1);
                oUv = uv * tc_extent + tc_center;
                tangent_quat = tangent_quat * 2 - 1;
                oX = transform_quat(float3(1, 0, 0), tangent_quat);
                oY = transform_quat(float3(0, 1, 0), tangent_quat) * sign(tangent_quat.w);
                oZ = transform_quat(float3(0, 0, 1), tangent_quat);
                oView = eye_pos_os - pos.xyz;
                oPos = mul(pos, worldviewproj);
            }
            float4 EditorLitPS(float2 uv : TEXCOORD0, float3 tx : TEXCOORD1, float3 ty : TEXCOORD2,
                float3 tz : TEXCOORD3, float3 view : TEXCOORD4) : SV_Target
            {
                float3 albedo = albedo_clr.rgb;
                if (albedo_map_enabled) albedo *= albedo_tex.Sample(linear_sampler, uv).rgb;
                float2 mg = metalness_glossiness_factor.xy;
                if (metalness_glossiness_factor.z > 0.5f)
                    mg *= get_xy_channel(metalness_glossiness_tex.Sample(linear_sampler, uv));
                mg = saturate(mg);
                float3 nt = float3(0, 0, 1);
                if (normal_map_enabled) {
                    nt = decompress_normal(normal_tex.Sample(linear_sampler, uv));
                    nt.xy *= normal_scale;
                    nt = normalize(nt);
                }
                float3 n = normalize(tx * nt.x + ty * nt.y + tz * nt.z);
                float3 l = normalize(float3(0.35f, 0.85f, 0.45f));
                float3 v = normalize(view);
                float3 h = normalize(l + v);
                float ndotl = saturate(dot(n, l));
                float shininess = exp2(13.0f * mg.y);
                float3 f0 = lerp(float3(0.04f, 0.04f, 0.04f), albedo, mg.x);
                float spec = pow(saturate(dot(n, h)), shininess) * (shininess + 2) / 8;
                float ao = 1;
                if (occlusion_map_enabled) ao = lerp(1, get_x_channel(occlusion_tex.Sample(linear_sampler, uv)), occlusion_strength);
                float3 emission = emissive_clr.rgb;
                if (emissive_clr.w > 0.5f) emission *= emissive_tex.Sample(linear_sampler, uv).rgb;
                float3 lit = albedo * (1 - mg.x) * (0.25f * ao + 0.75f * ndotl) + f0 * (spec * ndotl + 0.1f * ao) + emission;
                return float4(lit, 1);
            }
            ENDHLSL
        }
    }
}
