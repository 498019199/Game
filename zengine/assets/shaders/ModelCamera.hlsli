// Parameters, resources and pipeline state are declared by the C++ effect.

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
