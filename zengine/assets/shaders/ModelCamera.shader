Shader "Lib/ModelCamera"
{
    Struct KlayGECameraInfo
    {
        Float4x4 model_view
        Float4x4 mvp
        Float4x4 inv_mv
        Float4x4 inv_mvp
        Float3 eye_pos
        Float3 forward_vec
        Float3 up_vec
    }

    CBuffer klayge_model
    {
        Float4x4 model
        Float4x4 inv_model
    }

    CBuffer klayge_camera
    {
        UInt num_cameras
        UInt4 camera_indices array_size="2"
        KlayGECameraInfo cameras array_size="8"
        Float4x4 prev_mvps array_size="8"
    }

    HLSLPROGRAM
uint InstanceIndex(uint instance_id)
{
    return instance_id / num_cameras;
}

uint CameraIndex(uint instance_id)
{
    return instance_id % num_cameras;
}

KlayGECameraInfo CameraFromInstance(uint instance_id)
{
    return cameras[CameraIndex(instance_id)];
}

uint RenderTargetIndex(uint camera_id)
{
    return camera_indices[camera_id / 4][camera_id % 4];
}
    ENDHLSL
}
