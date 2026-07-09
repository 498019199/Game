Shader "Demo/SkyBox"
{
    // Pull decode_hdr_yc helpers (converted from util.fxml)
    Include "util.shader"

    Properties
    {
        inv_mvp ("InvMVP", Matrix) = {}
        depth_far ("DepthFar", Float) = 0
        skybox_compressed ("Compressed", Int) = 0
        skybox_tex ("Skybox", Cube) = "" {}
        skybox_C_tex ("SkyboxC", Cube) = "" {}
    }

    SubShader
    {
        // Deferred GBuffer stub (kept for API parity with SkyBox.fxml)
        Pass
        {
            Name "GBufferSkyBoxTech"
            Cull Off
            ZWrite Off
            ZTest Equal

            HLSLPROGRAM
            #pragma vertex GBufferSkyBoxVS
            #pragma fragment GBufferSkyBoxPS

            void GBufferSkyBoxVS(float4 pos : POSITION,
                        out float4 oPos : SV_Position)
            {
                oPos = pos;
            }

            void GBufferSkyBoxPS(out float4 rt0 : SV_Target0, out float4 rt1 : SV_Target1)
            {
                rt0 = rt1 = 0;
            }
            ENDHLSL
        }

        // Forward / simple path used by RenderableSkyBox today
        Pass
        {
            Name "SkyBoxTech"
            Cull Off
            ZWrite Off
            ZTest Equal

            HLSLPROGRAM
            #pragma vertex SkyBoxVS
            #pragma fragment SkyBoxPS

            void SkyBoxVS(float4 pos : POSITION,
                        out float3 texcoord0 : TEXCOORD0,
                        out float4 oPos : SV_Position)
            {
                oPos = pos;
                texcoord0 = mul(pos, inv_mvp).xyz;
            }

            float4 SkyBoxPS(float3 texCoord0 : TEXCOORD0) : SV_Target
            {
                if (skybox_compressed)
                {
                    return decode_hdr_yc(skybox_tex.Sample(skybox_sampler, texCoord0).r,
                                skybox_C_tex.Sample(skybox_sampler, texCoord0));
                }
                else
                {
                    return skybox_tex.Sample(skybox_sampler, texCoord0);
                }
            }
            ENDHLSL
        }
    }
}
