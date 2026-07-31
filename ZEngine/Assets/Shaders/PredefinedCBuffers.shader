Shader "PredefinedCBuffers"
{
    Include "Material.shader"
    Include "Mesh.shader"
    Include "ModelCamera.shader"

    SubShader
    {
        Pass
        {
            Name "PredefinedCBuffersNoopTech"

            HLSLPROGRAM
            #pragma vertex PredefinedCBuffersNoopVS
            #pragma fragment PredefinedCBuffersNoopPS

            // Noop pass: only exists so predefined cbuffers (mesh/model/material/camera)
            // are reflected into .kfx. Reference every member so reflection keeps full
            // layouts even when the compiler optimizes (offsets used by Predefined*CBuffer).
            void PredefinedCBuffersNoopVS(out float4 oPosition : SV_Position)
            {
                float3 pos = pos_center * pos_extent;
                float2 tc = tc_center * tc_extent;
                float4x4 world = mul(model, inv_model);
                float4 world_pos = mul(float4(pos + float3(tc, 0), 1), world);
                KlayGECameraInfo camera = CameraFromInstance(0);
                oPosition = mul(world_pos, camera.mvp);
                oPosition += float4(camera.eye_pos, 0) * 0;
                oPosition += float4(prev_mvps[0][0].xyz, 0) * 0;
                oPosition.w += (float)num_cameras * 0;
                oPosition.w += (float)camera_indices[0].x * 0;
            }

            float4 PredefinedCBuffersNoopPS() : SV_Target0
            {
                float4 c = albedo_clr;
                c.rgb *= metalness_glossiness_factor;
                c += emissive_clr;
                c.a += (float)(albedo_map_enabled + normal_map_enabled + height_map_parallax_enabled
                    + height_map_tess_enabled + occlusion_map_enabled) * 0;
                c.a += alpha_test_threshold * 0 + normal_scale * 0 + occlusion_strength * 0;
                c.xy += height_offset_scale * 0;
                c += tess_factors * 0;
                return c;
            }
            ENDHLSL
        }
    }
}
