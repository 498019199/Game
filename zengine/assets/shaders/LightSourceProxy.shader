Shader "LightSourceProxy"
{
    Include "Lighting.shader"
    Include "GBuffer.shader"

    Int2 light_is_projective

    Texture2D projective_map_2d_tex
    TextureCube projective_map_cube_tex

    Sampler linear_sampler
    {
        State filtering = min_mag_linear_mip_point
        State address_u = clamp
        State address_v = clamp
    }

    SubShader
    {
        Pass
        {
            Name "LightSourceProxy"
            Cull Back

            HLSLPROGRAM
            #pragma vertex LightSourceProxyVS
            #pragma fragment LightSourceProxyPS

            void LightSourceProxyVS(uint instance_id : SV_InstanceID,
                        float4 pos : POSITION,
                        float4 tangent_quat : TANGENT,
                out float3 oPosOS : TEXCOORD0,
                out float3 oNormal : TEXCOORD1,
                out float4 oPos : SV_Position)
            {
                pos = float4(pos.xyz * pos_extent + pos_center, 1);
                tangent_quat = tangent_quat * 2 - 1;

                KlayGECameraInfo camera = CameraFromInstance(instance_id);
                float4x4 mvp = camera.mvp;

                oPos = mul(pos, mvp);
                oPosOS = pos.xyz;
                oNormal = mul(transform_quat(float3(0, 0, 1), tangent_quat), (float3x3)model);
            }

            float4 LightSourceProxyPS(float3 pos_os : TEXCOORD0, float3 normal : TEXCOORD1) : SV_Target
            {
                float4 clr = albedo_clr;
                if (light_is_projective.x)
                {
                    float4 proj_clr;
                    if (light_is_projective.y)
                    {
                        proj_clr = projective_map_cube_tex.Sample(linear_sampler, pos_os);
                    }
                    else
                    {
                        float2 tc = pos_os.xy;
                        tc.y *= KLAYGE_FLIPPING;
                        proj_clr = projective_map_2d_tex.Sample(linear_sampler, tc * 0.5f + 0.5f);
                    }
                    clr *= proj_clr;
                }
                return (saturate(dot(normalize(normal), float3(0, 1, 0))) + 0.2f) * clr;
            }
            ENDHLSL
        }
    }
}
