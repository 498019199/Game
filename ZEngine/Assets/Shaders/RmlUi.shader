Shader "RmlUi"
{
    Float4x4 Transform
    Texture2D rml_tex

    Sampler rml_sampler
    {
        State filtering = min_mag_mip_linear
        State address_u = clamp
        State address_v = clamp
    }

    SubShader
    {
        Pass
        {
            Name "RmlUiTech"
            Cull Off
            ZWrite Off
            ZTest Always
            Blend One OneMinusSrcAlpha

            HLSLPROGRAM
            #pragma vertex RmlUiVS
            #pragma fragment RmlUiPS

            void RmlUiVS(float2 pos : POSITION,
                        float4 col : COLOR,
                        float2 uv : TEXCOORD0,
                out float4 oCol : COLOR,
                out float2 oUv : TEXCOORD0,
                out float4 oPos : SV_Position)
            {
                oPos = mul(float4(pos, 0.0f, 1.0f), Transform);
                oCol = col;
                oUv = uv;
            }

            float4 RmlUiPS(float4 col : COLOR, float2 uv : TEXCOORD0) : SV_Target
            {
                return col * rml_tex.Sample(rml_sampler, uv);
            }
            ENDHLSL
        }

        Pass
        {
            Name "RmlUiScissorTech"
            Cull Off
            ZWrite Off
            ZTest Always
            Blend One OneMinusSrcAlpha
            Scissor On

            HLSLPROGRAM
            #pragma vertex RmlUiScissorVS
            #pragma fragment RmlUiScissorPS

            void RmlUiScissorVS(float2 pos : POSITION,
                        float4 col : COLOR,
                        float2 uv : TEXCOORD0,
                out float4 oCol : COLOR,
                out float2 oUv : TEXCOORD0,
                out float4 oPos : SV_Position)
            {
                oPos = mul(float4(pos, 0.0f, 1.0f), Transform);
                oCol = col;
                oUv = uv;
            }

            float4 RmlUiScissorPS(float4 col : COLOR, float2 uv : TEXCOORD0) : SV_Target
            {
                return col * rml_tex.Sample(rml_sampler, uv);
            }
            ENDHLSL
        }
    }
}
