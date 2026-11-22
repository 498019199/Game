// Parameters, resources and pipeline state are declared by the C++ effect.

void TerrainVS(float4 pos : POSITION,
            float2 tex : TEXCOORD0,
            out float4 oTex : TEXCOORD0,
            out float oDepth : TEXCOORD1,
            out float4 oPos : SV_Position)
{
    pos = float4(pos.xyz * pos_extent + pos_center, 1);

    oTex.xy = float2(tex.x, 1 - tex.y);
    oTex.zw = oTex.xy * 8;

    pos = pos.xzyw;
    pos.y += height_map_tex.SampleLevel(point_sampler, oTex.xy, 0).r;

    oPos = mul(pos, mvp);
    oDepth = oPos.w;
}

float4 TerrainPS(float4 tex : TEXCOORD0, float depth : TEXCOORD1) : SV_Target
{
    float3 normal = normalize(decompress_normal(normal_map_tex.Sample(normal_map_sampler, tex.xy)).xzy * float3(4, 1, 4));
    return float4(grass_tex.Sample(trilinear_sampler, tex.zw).rgb * dot(normalize(LightDir), normal),
        depth * inv_far);
}




float4 BlendPS(float2 tex : TEXCOORD0) : SV_Target
{
    float4 fog = tex_with_alpha.Sample(point_sampler, tex);
    float clr = max(0, dot(LightDir, fog.xyz) / sqrt(dot(LightDir, LightDir) * dot(fog.xyz, fog.xyz)));
    fog.rgb = clr;
    fog.a = saturate(fog.a);
    float4 s = src_tex.Sample(point_sampler, tex);
    return float4(lerp(s.rgb, fog.rgb, fog.a), 1);
}
