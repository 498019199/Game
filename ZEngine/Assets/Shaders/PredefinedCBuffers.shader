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

            void PredefinedCBuffersNoopVS(out float4 oPositionOS : PositionOS, out float4 oPosition : SV_Position)
            {
                oPositionOS = mul(float4(pos_center, 1), model);

                KlayGECameraInfo camera = CameraFromInstance(0);
                float4x4 mvp = camera.mvp;
                oPosition = mul(float4(pos_center, 1), mvp);
            }

            float4 PredefinedCBuffersNoopPS() : SV_Target0
            {
                return albedo_clr;
            }
            ENDHLSL
        }
    }
}
