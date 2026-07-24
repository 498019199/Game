Shader "SimpleAlbedoNormal"
{
    // Simple forward preview:
    //   VS — packed mesh transform (pos / uv / tangent)
    //   PS — sample albedo + normal maps, fuse with Lambert lighting
    Include "util.shader"
    Include "Quaternion.shader"
    Include "Material.shader"
    Include "Mesh.shader"

    Float4x4 worldviewproj semantic="WORLDVIEWPROJECTION"
    Float3 light_dir = { 0.35, 0.55, 0.75 }
    Float normal_blend = 0.0

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

                // Object-space -> tangent-space basis (same layout as SubSurface).
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
                float3 albedo = albedo_clr.rgb;
                if (albedo_map_enabled)
                {
                    albedo *= albedo_tex.Sample(linear_sampler, uv).rgb;
                }

                float3 N = float3(0, 0, 1);
                if (normal_map_enabled)
                {
                    N = decompress_normal(normal_tex.Sample(linear_sampler, uv));
                    N.xy *= normal_scale;
                    N = normalize(N);
                }

                // Light in tangent space.
                float3x3 obj_to_ts = float3x3(ts_x, ts_y, ts_z);
                float3 L = normalize(mul(obj_to_ts, normalize(light_dir)));
                float ndotl = saturate(dot(N, L));

                // Fuse: albedo shaded by normal; optional normal-as-color debug mix.
                float3 lit = albedo * (0.25f + 0.75f * ndotl);
                float3 n_vis = N * 0.5f + 0.5f;
                float3 clr = lerp(lit, n_vis * albedo, saturate(normal_blend));

                return float4(clr, albedo_clr.a);
            }
            ENDHLSL
        }
    }
}
