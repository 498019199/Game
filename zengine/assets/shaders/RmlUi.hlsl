// Parameters, resources and pipeline state are declared by the C++ effect.

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
