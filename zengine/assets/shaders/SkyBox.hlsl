// Parameters, resources and pipeline state are declared by the C++ effect.

void GBufferSkyBoxVS(float4 pos : POSITION,
            out float4 oPos : SV_Position)
{
    oPos = pos;
}

void GBufferSkyBoxPS(out float4 rt0 : SV_Target0, out float4 rt1 : SV_Target1)
{
    rt0 = rt1 = 0;
}




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
