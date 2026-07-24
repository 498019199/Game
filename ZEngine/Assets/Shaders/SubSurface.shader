Shader "SubSurface"
{
    Include "Lighting.shader"
    Include "util.shader"
    Include "Quaternion.shader"
    Include "Material.shader"
    Include "Mesh.shader"

    Float4x4 worldviewproj semantic="WORLDVIEWPROJECTION"
    Float3 eye_pos
    Float3 light_pos
    Float3 light_color
    Float3 light_falloff
    Float3 extinction_coefficient
    Float material_thickness
    Float sigma_t
    Float2 far_plane
    Float2 near_q

    Texture2D back_face_depth_tex

    Sampler linear_sampler
    {
        State filtering = min_mag_mip_linear
        State address_u = wrap
        State address_v = wrap
    }

    Sampler point_sampler
    {
        State filtering = min_mag_mip_point
        State address_u = clamp
        State address_v = clamp
    }

    SubShader
    {
        Pass
        {
            Name "BackFaceDepthTechWODepthTexture"
            Cull Front
            ZTest Greater

            HLSLPROGRAM
            #pragma vertex BackFaceDepthVS
            #pragma fragment BackFaceDepthPS

            void BackFaceDepthVS(float2 tex0 : TEXCOORD0,
                        float4 pos : POSITION,
                        out float oDepth : TEXCOORD0,
                        out float4 oPos : SV_Position)
            {
                pos = float4(pos.xyz * pos_extent + pos_center, 1);
                oPos = mul(pos, worldviewproj);
                oDepth = oPos.w;
            }

            float4 BackFaceDepthPS(float depth : TEXCOORD0) : SV_Target
            {
                return WriteAFloat(depth, far_plane.y);
            }
            ENDHLSL
        }

        Pass
        {
            Name "BackFaceDepthTech"
            Cull Front
            ZTest Greater
            State color_write_mask = 0

            HLSLPROGRAM
            #pragma vertex BackFaceDepthVS
            #pragma fragment BackFaceDepthPS
            ENDHLSL
        }

        Pass
        {
            Name "SubSurfaceTech"
            Cull Back

            HLSLPROGRAM
            #pragma vertex SubSurfaceVS
            #pragma fragment SubSurfacePS

            void SubSurfaceVS(float2 tex0 : TEXCOORD0,
                        float4 pos : POSITION,
                        float4 tangent_quat : TANGENT,
                        out float2 oTex0 : TEXCOORD0,
                        out float3 oL : TEXCOORD1,
                        out float3 oV : TEXCOORD2,
                        out float3 oH : TEXCOORD3,
                        out float3 oTcW : TEXCOORD4,
                        out float4 oPos : SV_Position)
            {
                pos = float4(pos.xyz * pos_extent + pos_center, 1);
                tex0 = tex0 * tc_extent + tc_center;
                tangent_quat = tangent_quat * 2 - 1;

                oTex0 = tex0;

                float3x3 obj_to_ts;
                obj_to_ts[0] = transform_quat(float3(1, 0, 0), tangent_quat);
                obj_to_ts[1] = transform_quat(float3(0, 1, 0), tangent_quat) * sign(tangent_quat.w);
                obj_to_ts[2] = transform_quat(float3(0, 0, 1), tangent_quat);

                float3 view_vec = eye_pos - pos.xyz;
                float3 light_vec = light_pos - pos.xyz;
                float3 halfway = normalize(light_vec) + normalize(view_vec);

                oL = mul(obj_to_ts, light_vec);
                oV = mul(obj_to_ts, view_vec);
                oH = mul(obj_to_ts, halfway);

                oPos = mul(pos, worldviewproj);

                oTcW.xy = oPos.xy / oPos.w;
                oTcW.y *= KLAYGE_FLIPPING;
                oTcW.xy = oTcW.xy * 0.5f + 0.5f;
                oTcW.z = oPos.w;
                oTcW.xy *= oTcW.z;
            }

            float4 SubSurfacePS(float2 uv : TEXCOORD0,
                        float3 L : TEXCOORD1,
                        float3 V : TEXCOORD2,
                        float3 H : TEXCOORD3,
                        float3 tc_w : TEXCOORD4) : SV_Target
            {
                float attenuation = AttenuationTerm(0, -L.xyz, light_falloff);

                L = normalize(L);
                V = normalize(V);
                H = normalize(H);
                float3 N;
                if (normal_map_enabled)
                {
                    N = decompress_normal(normal_tex.Sample(linear_sampler, uv));
                }
                else
                {
                    N = float3(0, 0, 1);
                }

                float3 albedo = albedo_clr.rgb;
                if (albedo_map_enabled)
                {
                    albedo *= albedo_tex.Sample(linear_sampler, uv).rgb;
                }

                float3 diffuse = DiffuseColor(albedo, 0.5f);
                float3 specular = SpecularColor(albedo, 0.5f);

                float glossiness = metalness_glossiness_factor.y;
                if (metalness_glossiness_factor.z > 0.5f)
                {
                    glossiness *= get_xy_channel(metalness_glossiness_tex.Sample(linear_sampler, uv)).y;
                }
                float shininess = Glossiness2Shininess(glossiness);

                float2 tex_coord = tc_w.xy / tc_w.z;
#if NO_DEPTH_TEXTURE
                float back_face_depth = ReadAFloat(back_face_depth_tex.Sample(point_sampler, tex_coord), far_plane.x);
#else
                float back_face_depth = back_face_depth_tex.Sample(point_sampler, tex_coord).x;
                // Unbound/cleared depth + unset near_q → 0/0 NaN blacks the whole pixel.
                float q = max(near_q.y, 1e-5f);
                back_face_depth = near_q.x / max(q - back_face_depth, 1e-5f);
#endif
                float thickness = clamp(back_face_depth - tc_w.z, 0, 100.0f);

                float indirect_light = material_thickness * (min(0, dot(N, L)) + min(0, dot(V, L)));
                indirect_light *= exp(thickness * sigma_t);

                float3 clr = (albedo_clr.rgb * 0.2f * diffuse + CalcBRDFShading(diffuse, specular, shininess, L, H, N)
                    + indirect_light * extinction_coefficient) * attenuation;

                return float4(clr.rgb * light_color, 1);
            }
            ENDHLSL
        }

        Pass
        {
            Name "SubSurfaceTechWODepthTexture"
            Cull Back
            Macro NO_DEPTH_TEXTURE = 1

            HLSLPROGRAM
            #pragma vertex SubSurfaceVS
            #pragma fragment SubSurfacePS
            ENDHLSL
        }
    }
}
