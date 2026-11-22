// Parameters, resources and pipeline state are declared by the C++ effect.

void CameraFrustumVS(uint instance_id : SV_InstanceID,
    float3 pos : POSITION,
    out float4 oPos : SV_Position)
{
    KlayGECameraInfo camera = CameraFromInstance(instance_id);
    oPos = mul(float4(pos, 1), camera.mvp);
}

float4 CameraFrustumPS() : SV_Target
{
    return albedo_clr;
}
