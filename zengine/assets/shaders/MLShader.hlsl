// Parameters, resources and pipeline state are declared by the C++ effect.

// Soft bloom weight from neighboring emissive mask samples.
float SampleEmissiveBloom(float2 uv, float radius)
{
    float r = max(radius, 1e-5f);
    // 8-tap ring + center (cheap UV blur of mask1.A)
    float c = mask1_tex.Sample(linear_sampler, uv).a;
    float s = c * 2.0f;
    s += mask1_tex.Sample(linear_sampler, uv + float2( r,  0)).a;
    s += mask1_tex.Sample(linear_sampler, uv + float2(-r,  0)).a;
    s += mask1_tex.Sample(linear_sampler, uv + float2( 0,  r)).a;
    s += mask1_tex.Sample(linear_sampler, uv + float2( 0, -r)).a;
    float d = r * 0.7071f;
    s += mask1_tex.Sample(linear_sampler, uv + float2( d,  d)).a;
    s += mask1_tex.Sample(linear_sampler, uv + float2(-d,  d)).a;
    s += mask1_tex.Sample(linear_sampler, uv + float2( d, -d)).a;
    s += mask1_tex.Sample(linear_sampler, uv + float2(-d, -d)).a;
    // Outer ring (wider halo)
    float r2 = r * 2.0f;
    s += mask1_tex.Sample(linear_sampler, uv + float2( r2,  0)).a * 0.5f;
    s += mask1_tex.Sample(linear_sampler, uv + float2(-r2,  0)).a * 0.5f;
    s += mask1_tex.Sample(linear_sampler, uv + float2( 0,  r2)).a * 0.5f;
    s += mask1_tex.Sample(linear_sampler, uv + float2( 0, -r2)).a * 0.5f;
    return s / 12.0f;
}

void SimpleAlbedoNormalVS(float2 tex0 : TEXCOORD0,
            float4 pos : POSITION,
            float4 tangent_quat : TANGENT,
            out float2 oTex0 : TEXCOORD0,
            out float3 oTsX : TEXCOORD1,
            out float3 oTsY : TEXCOORD2,
            out float3 oTsZ : TEXCOORD3,
            out float4 oPos : SV_Position)
{
    pos = float4(pos.xyz * pos_extent + pos_center, 1);
    tex0 = tex0 * tc_extent + tc_center;
    tangent_quat = tangent_quat * 2 - 1;

    oTex0 = tex0;
    oTsX = transform_quat(float3(1, 0, 0), tangent_quat);
    oTsY = transform_quat(float3(0, 1, 0), tangent_quat) * sign(tangent_quat.w);
    oTsZ = transform_quat(float3(0, 0, 1), tangent_quat);
    oPos = mul(pos, worldviewproj);
}

float4 SimpleAlbedoNormalPS(float2 uv : TEXCOORD0,
            float3 ts_x : TEXCOORD1,
            float3 ts_y : TEXCOORD2,
            float3 ts_z : TEXCOORD3) : SV_Target
{
    float4 albedo_s = albedo_tex.Sample(linear_sampler, uv);
    float4 normal_s = normal_tex.Sample(linear_sampler, uv);

    float3 albedo = albedo_s.rgb;
    if (dot(albedo, albedo) < 1e-6f)
    {
        albedo = max(albedo_clr.rgb, float3(0.6f, 0.6f, 0.6f));
    }

    float3 N = float3(0, 0, 1);
    if (normal_map_enabled)
    {
        N = decompress_normal(normal_s);
        N.xy *= normal_scale;
        N = normalize(N);
    }

    float3x3 obj_to_ts = float3x3(ts_x, ts_y, ts_z);
    float3 L = normalize(mul(obj_to_ts, normalize(light_dir)));
    float ndotl = saturate(dot(N, L));

    float3 lit = albedo * (0.45f + 0.55f * ndotl);

    if (mask1_map_enabled)
    {
        float4 mask1 = mask1_tex.Sample(linear_sampler, uv);
        float emask = saturate(mask1.a);

        // Core self-illum
        float3 glow_rgb = albedo * emissive_color * selfillum_strength;
        lit += emask * glow_rgb;

        // Fake bloom / 泛光: soft UV halo from neighboring emissive
        float bloom = SampleEmissiveBloom(uv, bloom_radius);
        bloom = max(bloom - bloom_threshold, 0.0f);
        lit += bloom * glow_rgb * bloom_strength;

        // Specular on emissive core
        float3 H = normalize(L + float3(0, 0, 1));
        float ndoth = saturate(dot(N, H));
        float spec = pow(ndoth, max(specular_power, 1.0f)) * emask * specular_strength;
        lit += specular_color * spec * max(emissive_color, float3(0.2f, 0.2f, 0.2f));
    }

    return float4(lit, albedo_s.a);
}
