// Parameters, resources and pipeline state are declared by the C++ effect.

void PositionNode(float3 pos, float4 tangent_quat, float4 blend_weights, int4 blend_indices, out float3 result_pos, out float4 result_tangent_quat);
float2 TexcoordNode(float2 texcoord);
float3 AlbedoNode(float2 texcoord);
float2 MetalnessGlossinessNode(float2 texcoord);
float3 EmissiveNode(float2 texcoord);
float OpacityNode(float2 texcoord);
float3 NormalNode(float2 texcoord);
float HeightForParallaxNode(float2 texcoord);
float HeightForParallaxOcclusionNode(float2 texcoord, int level);
float HeightForTessellationNode(float2 texcoord);
float OcclusionNode(float2 texcoord);
float3 PositionAdjustmentNode(float3 pos, float4 tangent_quat);
float2 TexcoordAdjustmentNode(float2 texcoord, float3 view_ray);
void GBufferVS(
#if MULTI_VIEW_MODE
			uint instance_id : SV_InstanceID,
#endif
			float4 pos : POSITION,
			float2 texcoord : TEXCOORD0,
			float4 tangent_quat : TANGENT,
#if NUM_JOINTS > 0
			float4 blend_weights : BLENDWEIGHT,
#if KLAYGE_OPENGL || KLAYGE_OPENGLES
			float4 blend_indices_f : BLENDINDICES,
#else
			uint4 blend_indices : BLENDINDICES,
#endif
#endif
			out float4 oTexCoord_2xy : TEXCOORD0,
			out float4 oTsToView0_2z : TEXCOORD1,
			out float4 oTsToView1_Depth : TEXCOORD2,
			out SS_TEXCOORD_DECL oScreenTc : TEXCOORD3,
			out float4 oCurrPosSS : TEXCOORD4,
			out float4 oPrevPosSS : TEXCOORD5,
#if MULTI_VIEW_MODE
#if KLAYGE_VP_RT_INDEX_AT_EVERY_STAGE_SUPPORT
			out uint oRtIndex : SV_RenderTargetArrayIndex,
			out float4 oPos : SV_Position
#else
			out float oRtIndex : TEXCOORD6,
			out float4 oPos : POSITION
#endif
#else
			out float4 oPos : SV_Position
#endif
			)
{
#if NUM_JOINTS > 0
#if KLAYGE_OPENGL || KLAYGE_OPENGLES
	int4 blend_indices = (int4)blend_indices_f;
#endif
#else
	float4 blend_weights = 0;
	int4 blend_indices = 0;
#endif

#if MULTI_VIEW_MODE
	uint camera_index = CameraIndex(instance_id);
#else
	uint camera_index = 0;
#endif
	KlayGECameraInfo camera = cameras[camera_index];
	float4x4 mvp = camera.mvp;
	float4x4 model_view = camera.model_view;

	pos = float4(pos.xyz * pos_extent + pos_center, 1);
	texcoord = texcoord * tc_extent + tc_center;
	tangent_quat = tangent_quat * 2 - 1;

	float3 result_pos;
	float4 result_tangent_quat;
	PositionNode(pos.xyz, tangent_quat, blend_weights, blend_indices, result_pos, result_tangent_quat);
	oTexCoord_2xy.xy = TexcoordNode(texcoord);

	oPos = mul(float4(result_pos, 1), mvp);

	float3x3 obj_to_ts;
	obj_to_ts[0] = transform_quat(float3(1, 0, 0), result_tangent_quat);
	obj_to_ts[1] = transform_quat(float3(0, 1, 0), result_tangent_quat) * sign(result_tangent_quat.w);
	obj_to_ts[2] = transform_quat(float3(0, 0, 1), result_tangent_quat);
	float3x3 ts_to_view = mul(obj_to_ts, (float3x3)model_view);
	oTsToView0_2z.xyz = ts_to_view[0];
	oTsToView1_Depth.xyz = ts_to_view[1];
	oTexCoord_2xy.zw = ts_to_view[2].xy;
	oTsToView0_2z.w = ts_to_view[2].z;

	oTsToView1_Depth.w = oPos.w;

	oScreenTc = EncodeSSTexcoord(oPos);

	oCurrPosSS = oPos;
	oPrevPosSS = mul(float4(result_pos, 1), prev_mvps[camera_index]);

	uint rt_index = RenderTargetIndex(camera_index);
#if MULTI_VIEW_MODE
#if KLAYGE_VP_RT_INDEX_AT_EVERY_STAGE_SUPPORT
	oRtIndex = rt_index;
#else
	oRtIndex = rt_index + 0.5f;
#endif
#endif
}

float2 ParallaxMappingCorrection(float2 uv, float3 view)
{
	uv += view.xy * HeightForParallaxNode(uv);
	return uv;
}

float3 RestoreNormal(float4 texcoord_2xy, float4 ts_to_view0_2z, float3 ts_to_view1)
{
	float3 normal = normalize(float3(texcoord_2xy.zw, ts_to_view0_2z.w));

	float3x3 ts_to_view;
	ts_to_view[0] = normalize(ts_to_view0_2z.xyz);
	ts_to_view[1] = normalize(ts_to_view1);
	ts_to_view[2] = normal;
	normal = normalize(mul(NormalNode(texcoord_2xy.xy) * float3(normal_scale.xx, 1), ts_to_view));

	return normal;
}

void ConstructGBuffer(float revert_normal, float4 texcoord_2xy, float4 ts_to_view0_2z, float3 ts_to_view1, float2 motion_vec,
					out float4 rt0, out float4 rt1, out float4 rt2)
{
	float3 normal = RestoreNormal(texcoord_2xy, ts_to_view0_2z, ts_to_view1) * revert_normal;

	float3 albedo = AlbedoNode(texcoord_2xy.xy);
	float2 metalness_glossiness = MetalnessGlossinessNode(texcoord_2xy.xy);

	float occlusion = OcclusionNode(texcoord_2xy.xy);

	StoreGBuffer(normal, albedo, metalness_glossiness, motion_vec, occlusion,
		rt0, rt1, rt2);
}

float2 GenerateVelocity(float4 curr_pos_ss, float4 prev_pos_ss, float half_exposure_x_framerate)
{
	const float EPSILON = 0.01f;

	float2 velocity = (curr_pos_ss.xy / curr_pos_ss.w - prev_pos_ss.xy / prev_pos_ss.w) * half_exposure_x_framerate;
	float len_velocity = length(velocity);

	float weight = max(0.5f, min(len_velocity, motion_blur_radius));
	weight /= max(len_velocity, EPSILON);
	velocity *= weight;
	velocity.y *= KLAYGE_FLIPPING;
	return velocity;
}

void GBufferPS(float4 texcoord_2xy : TEXCOORD0, float4 ts_to_view0_2z : TEXCOORD1, float4 ts_to_view1_depth : TEXCOORD2,
					SS_TEXCOORD_DECL ss_tc : TEXCOORD3,
					float4 curr_pos_ss : TEXCOORD4, float4 prev_pos_ss : TEXCOORD5,
					bool is_front_face : SV_IsFrontFace,
					out float4 rt0 : SV_Target0, out float4 rt1 : SV_Target1
#if !SKIP_MOTION_VEC
					, out float4 rt2 : SV_Target2
#endif
	)
{
	float3 view = -normalize(float3(ts_to_view0_2z.z, ts_to_view1_depth.z, ts_to_view0_2z.w));
	texcoord_2xy.xy = TexcoordAdjustmentNode(texcoord_2xy.xy, view);

#if ALPHA_TEST_MODE
	float opacity = OpacityNode(texcoord_2xy.xy);
	clip(opacity - alpha_test_threshold);
#endif

#if !SKIP_MOTION_VEC
	float2 motion_vec = GenerateVelocity(curr_pos_ss, prev_pos_ss, half_exposure_x_framerate);
#else
	float2 motion_vec = 0;
	float4 rt2;
#endif

	ConstructGBuffer(is_front_face ? 1 : -1, texcoord_2xy, ts_to_view0_2z, ts_to_view1_depth.xyz, motion_vec, rt0, rt1, rt2);
}

void GBufferAlphaBlendPS(float4 texcoord_2xy : TEXCOORD0, float4 ts_to_view0_2z : TEXCOORD1, float4 ts_to_view1_depth : TEXCOORD2,
					SS_TEXCOORD_DECL ss_tc : TEXCOORD3,
					float4 curr_pos_ss : TEXCOORD4, float4 prev_pos_ss : TEXCOORD5,
					out float4 rt0 : SV_Target0, out float4 rt1 : SV_Target1
#if !SKIP_MOTION_VEC
					, out float4 rt2 : SV_Target2
#endif
	)
{
	float2 screen_tc = DecodeSSTexcoord(ss_tc);
	clip(opaque_depth_tex.Sample(point_sampler, screen_tc).x - ts_to_view1_depth.w);

	GBufferPS(texcoord_2xy, ts_to_view0_2z, ts_to_view1_depth, ss_tc, curr_pos_ss, prev_pos_ss, true,
		rt0, rt1
#if !SKIP_MOTION_VEC
		, rt2
#endif
		);
}

void GenShadowMapVS(
#if MULTI_VIEW_MODE
						uint instance_id : SV_InstanceID,
#endif
						float4 pos : POSITION,
						float2 texcoord : TEXCOORD0,
						float4 tangent_quat : TANGENT,
#if NUM_JOINTS > 0
						float4 blend_weights : BLENDWEIGHT,
#if KLAYGE_OPENGL || KLAYGE_OPENGLES
						float4 blend_indices_f : BLENDINDICES,
#else
						uint4 blend_indices : BLENDINDICES,
#endif
#endif
						out float3 oTc : TEXCOORD0,
#if MULTI_VIEW_MODE
#if KLAYGE_VP_RT_INDEX_AT_EVERY_STAGE_SUPPORT
						out uint oRtIndex : SV_RenderTargetArrayIndex,
						out float4 oPos : SV_Position
#else
						out float oRtIndex : TEXCOORD1,
						out float4 oPos : POSITION
#endif
#else
						out float4 oPos : SV_Position
#endif
						)
{
#if NUM_JOINTS > 0
#if KLAYGE_OPENGL || KLAYGE_OPENGLES
	int4 blend_indices = (int4)blend_indices_f;
#endif
#else
	float4 blend_weights = 0;
	int4 blend_indices = 0;
#endif

#if MULTI_VIEW_MODE
	uint camera_index = CameraIndex(instance_id);
#else
	uint camera_index = 0;
#endif
	KlayGECameraInfo camera = cameras[camera_index];
	float4x4 mvp = camera.mvp;
	float4x4 model_view = camera.model_view;

	pos = float4(pos.xyz * pos_extent + pos_center, 1);
	texcoord = texcoord * tc_extent + tc_center;
	tangent_quat = tangent_quat * 2 - 1;

	float3 result_pos;
	float4 result_tangent_quat;
	PositionNode(pos.xyz, tangent_quat, blend_weights, blend_indices, result_pos, result_tangent_quat);
	result_pos = PositionAdjustmentNode(result_pos, result_tangent_quat);
	oTc.xy = TexcoordNode(texcoord);

	oPos = mul(float4(result_pos, 1), mvp);
	oTc.z = mul(float4(result_pos, 1), model_view).z;

	uint rt_index = RenderTargetIndex(camera_index);
#if MULTI_VIEW_MODE
#if KLAYGE_VP_RT_INDEX_AT_EVERY_STAGE_SUPPORT
	oRtIndex = rt_index;
#else
	oRtIndex = rt_index + 0.5f;
#endif
#endif
}

float4 GenShadowMapPS(float3 tc : TEXCOORD0) : SV_Target
{
	return tc.z;
}

float4 GenShadowMapAlphaTestPS(float3 tc : TEXCOORD0) : SV_Target
{
	float opacity = OpacityNode(tc.xy);
	clip(opacity - alpha_test_threshold);
	return tc.z;
}

float4 SpecialShadingPS(float2 texcoord : TEXCOORD0) : SV_Target
{
	float3 emissive = EmissiveNode(texcoord);
	return float4(max(emissive, 0), 1);
}

float4 SpecialShadingAlphaBlendPS(float2 texcoord : TEXCOORD0) : SV_Target
{
	float4 shading = SpecialShadingPS(texcoord);

	float opacity = OpacityNode(texcoord);
	return float4(shading.xyz, opacity);
}

float4 SelectModePS() : SV_Target
{
	return object_id;
}

void StaticPositionNode(float3 pos, float4 tangent_quat, float4 blend_weights, int4 blend_indices,
	out float3 result_pos, out float4 result_tangent_quat)
{
	result_pos = pos.xyz;
	result_tangent_quat = normalize(tangent_quat);
}

float2 StaticTexcoordNode(float2 texcoord)
{
	return texcoord;
}

float3 StaticAlbedoNode(float2 texcoord)
{
	float3 albedo = albedo_clr.rgb;
	if (albedo_map_enabled)
	{
		albedo *= albedo_tex.Sample(aniso_sampler, texcoord).rgb;
	}
	return albedo;
}

float2 StaticMetalnessGlossinessNode(float2 texcoord)
{
	float2 metalness_glossiness = metalness_glossiness_factor.xy;
	if (metalness_glossiness_factor.z > 0.5f)
	{
		metalness_glossiness *= get_xy_channel(metalness_glossiness_tex.Sample(aniso_sampler, texcoord));
	}
	return metalness_glossiness;
}

float3 StaticEmissiveNode(float2 texcoord)
{
	float3 emissive = emissive_clr.rgb;
	if (emissive_clr.w > 0.5f)
	{
		emissive *= emissive_tex.Sample(aniso_sampler, texcoord).rgb;
	}
	return emissive;
}

float StaticOpacityNode(float2 texcoord)
{
	float opacity = albedo_clr.a;
	if (albedo_map_enabled)
	{
		opacity *= albedo_tex.Sample(bilinear_sampler, texcoord).a;
	}
	return opacity;
}

float3 StaticNormalNode(float2 texcoord)
{
	float3 normal = float3(0, 0, 1);
	if (normal_map_enabled)
	{
		normal = decompress_normal(normal_tex.Sample(trilinear_sampler, texcoord));
	}
	return normal;
}

float StaticHeightForParallaxNode(float2 texcoord)
{
	float height = 0;
	if (height_map_parallax_enabled)
	{
		height = (height_tex.Sample(trilinear_sampler, texcoord).x + height_offset_scale.x) * height_offset_scale.y;
	}
	return height;
}

float StaticHeightForParallaxOcclusionNode(float2 texcoord, int level)
{
	float height = 0;
	if (height_map_parallax_enabled)
	{
		height = height_tex.SampleLevel(trilinear_sampler, texcoord, level).x;
	}
	return height;
}

float StaticHeightForTessellationNode(float2 texcoord)
{
	float height = 0;
	if (height_map_tess_enabled)
	{
		height = (height_tex.SampleLevel(bilinear_sampler, texcoord, 0).x + height_offset_scale.x) * 0.1f * height_offset_scale.y;
	}
	return height;
}

float StaticOcclusionNode(float2 texcoord)
{
	float occlusion = 1;
	if (occlusion_map_enabled)
	{
		occlusion = get_x_channel(occlusion_tex.Sample(trilinear_sampler, texcoord));
	}
	return occlusion;
}

float3 DefaultPositionAdjustmentNode(float3 position, float4 tangent_quat)
{
	return position;
}

float2 DefaultTexcoordAdjustmentNode(float2 texcoord, float3 view_ray)
{
	return ParallaxMappingCorrection(texcoord, view_ray);
}

#if KLAYGE_SHADER_MODEL >= SHADER_MODEL(4, 0)
bool FrustumCulling(float4 pos0, float4 pos1, float4 pos2)
{
	float4 t0 = saturate(pos0.xyxy * float4(-1, -1, 1, 1) - pos0.w);
	float4 t1 = saturate(pos1.xyxy * float4(-1, -1, 1, 1) - pos1.w);
	float4 t2 = saturate(pos2.xyxy * float4(-1, -1, 1, 1) - pos2.w);
	float4 t = t0 * t1 * t2;
	return !any(t);
}

struct GBufferPS_In
{
	float4 texcoord_2xy : TEXCOORD0;
	float4 ts_to_view0_2z : TEXCOORD1;
	float3 ts_to_view1 : TEXCOORD2;
	SS_TEXCOORD_DECL ss_tc : TEXCOORD3;
	float4 curr_pos_ss : TEXCOORD4;
	float4 prev_pos_ss : TEXCOORD5;
	uint rt_index : SV_RenderTargetArrayIndex;
	float4 position : SV_Position;
};

[maxvertexcount(3)]
void GBufferNoVpRtGS(triangle float4 in_texcoord_2xy[3] : TEXCOORD0, triangle float4 in_ts_to_view0_2z[3] : TEXCOORD1,
	triangle float3 in_ts_to_view1[3] : TEXCOORD2, triangle SS_TEXCOORD_TYPE in_ss_tc[3] : TEXCOORD3,
	triangle float4 in_curr_pos_ss[3] : TEXCOORD4, triangle float4 in_prev_pos_ss[3] : TEXCOORD5,
	triangle float in_rt_index[3] : TEXCOORD6, triangle float4 in_position[3] : POSITION,
	inout TriangleStream<GBufferPS_In> out_stream)
{
	GBufferPS_In output;
	output.rt_index = (uint)in_rt_index[0];

	[branch]
	if (FrustumCulling(in_position[0], in_position[1], in_position[2]))
	{
		[unroll]
		for (int v = 0; v < 3; ++ v)
		{
			output.texcoord_2xy = in_texcoord_2xy[v];
			output.ts_to_view0_2z = in_ts_to_view0_2z[v];
			output.ts_to_view1 = in_ts_to_view1[v];
			output.ss_tc = in_ss_tc[v];
			output.curr_pos_ss = in_curr_pos_ss[v];
			output.prev_pos_ss = in_prev_pos_ss[v];
			output.position = in_position[v];
			out_stream.Append(output);
		}
	}
}
#endif

#if KLAYGE_SHADER_MODEL >= SHADER_MODEL(4, 0)
struct GenShadowMapPS_In
{
	float3 tc : TEXCOORD0;
	uint rt_index : SV_RenderTargetArrayIndex;
	float4 position : SV_Position;
};

[maxvertexcount(3)]
void GenShadowCubemapNoVpRtGS(triangle float3 in_tc[3] : TEXCOORD0, triangle float in_rt_index[3] : TEXCOORD1,
	triangle float4 in_position[3] : POSITION, inout TriangleStream<GenShadowMapPS_In> out_stream)
{
	GenShadowMapPS_In output;
	output.rt_index = (uint)in_rt_index[0];

	[branch]
	if (FrustumCulling(in_position[0], in_position[1], in_position[2]))
	{
		[unroll]
		for (int v = 0; v < 3; ++ v)
		{
			output.tc = in_tc[v];
			output.position = in_position[v];
			out_stream.Append(output);
		}
	}
}
#endif

void PositionNode(float3 pos, float4 tangent_quat, float4 blend_weights, int4 blend_indices, out float3 result_pos, out float4 result_tangent_quat)
{
    StaticPositionNode(pos, tangent_quat, blend_weights, blend_indices, result_pos, result_tangent_quat);
}

float2 TexcoordNode(float2 texcoord)
{
    return StaticTexcoordNode(texcoord);
}

float3 AlbedoNode(float2 texcoord)
{
    return StaticAlbedoNode(texcoord);
}

float2 MetalnessGlossinessNode(float2 texcoord)
{
    return StaticMetalnessGlossinessNode(texcoord);
}

float3 EmissiveNode(float2 texcoord)
{
    return StaticEmissiveNode(texcoord);
}

float OpacityNode(float2 texcoord)
{
    return StaticOpacityNode(texcoord);
}

float3 NormalNode(float2 texcoord)
{
    return StaticNormalNode(texcoord);
}

float HeightForParallaxNode(float2 texcoord)
{
    return StaticHeightForParallaxNode(texcoord);
}

float HeightForParallaxOcclusionNode(float2 texcoord, int level)
{
    return StaticHeightForParallaxOcclusionNode(texcoord, level);
}

float HeightForTessellationNode(float2 texcoord)
{
    return StaticHeightForTessellationNode(texcoord);
}

float OcclusionNode(float2 texcoord)
{
    return StaticOcclusionNode(texcoord);
}

float3 PositionAdjustmentNode(float3 pos, float4 tangent_quat)
{
    return DefaultPositionAdjustmentNode(pos, tangent_quat);
}

float2 TexcoordAdjustmentNode(float2 texcoord, float3 view_ray)
{
    return DefaultTexcoordAdjustmentNode(texcoord, view_ray);
}
