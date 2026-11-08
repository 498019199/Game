Shader "CameraProxy"
{
    Include "Quaternion.shader"
    Include "Material.shader"
    Include "Mesh.shader"
    Include "ModelCamera.shader"

    SubShader
    {
        Pass
        {
            Name "CameraProxy"
            Cull Back

            HLSLPROGRAM
            #pragma vertex CameraProxyVS
            #pragma fragment CameraProxyPS

            void CameraProxyVS(uint instance_id : SV_InstanceID,
                        float4 pos : POSITION,
                        float4 tangent_quat : TANGENT,
                out float3 oNormal : TEXCOORD0,
                out float4 oPos : SV_Position)
            {
                pos = float4(pos.xyz * pos_extent + pos_center, 1);
                tangent_quat = tangent_quat * 2 - 1;

                KlayGECameraInfo camera = CameraFromInstance(instance_id);
                float4x4 mvp = camera.mvp;

                oPos = mul(pos, mvp);
                oNormal = mul(transform_quat(float3(0, 0, 1), tangent_quat), (float3x3)model);
            }

            float4 CameraProxyPS(float3 normal : TEXCOORD0) : SV_Target
            {
                float4 clr = albedo_clr;
                return (saturate(dot(normalize(normal), float3(0, 1, 0))) + 0.2f) * clr;
            }
            ENDHLSL
        }
    }
}
