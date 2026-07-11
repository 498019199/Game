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
            // are reflected into .kfx. Keep semantics D3D11-valid (SV_* / TEXCOORD).
            void PredefinedCBuffersNoopVS(out float4 oPosition : SV_Position)
            {
                float4 world_pos = mul(float4(pos_center, 1), model);
                KlayGECameraInfo camera = CameraFromInstance(0);
                oPosition = mul(world_pos, camera.mvp);
            }

            float4 PredefinedCBuffersNoopPS() : SV_Target0
            {
                return albedo_clr;
            }
            ENDHLSL
        }
    }
}
