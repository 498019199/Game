Shader "RenderFX/PredefinedCBuffers"
{
    // Converted from RenderFX/PredefinedCBuffers.fxml
    // Full effect XML embedded for 1:1 runtime compatibility.
    FXMLPROGRAM
<effect>
	<include name="Material.shader"/>
	<include name="Mesh.shader"/>
	<include name="ModelCamera.shader"/>

	<shader>
		<![CDATA[
// Just dummy shaders

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
		]]>
	</shader>

	<technique name="PredefinedCBuffersNoopTech">
		<pass name="p0">
			<state name="vertex_shader" value="PredefinedCBuffersNoopVS()"/>
			<state name="pixel_shader" value="PredefinedCBuffersNoopPS()"/>
		</pass>
	</technique>
</effect>
    ENDFXML
}
