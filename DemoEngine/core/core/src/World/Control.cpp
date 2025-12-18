#include <world/Control.h>
#include <world/SceneNode.h>

namespace RenderWorker
{
Controller::Controller()
    :rotationScaler_(0.05f), moveScaler_(1), camera_(nullptr)
{
    
}

void Controller::Scalers(float rotationScaler, float moveScaler)
{
    rotationScaler_ = rotationScaler;
    moveScaler_ = moveScaler;
}

void Controller::AttachCamera(const CameraPtr&  camera)
{
    camera_ = camera;
}

void Controller::DetachCamera()
{
    camera_ = nullptr;
}

    


FirstPersonController::FirstPersonController()
{
}

FirstPersonController::~FirstPersonController() noexcept  = default;

void FirstPersonController::AttachCamera(const CameraPtr& camera)
{
    float3 scale;
    float3 translation;
    quater quat;
    MathWorker::decompose(scale, quat, translation, camera->ViewMatrix());

    rotator rot = MathWorker::ToRotator(quat);

    MathWorker::sincos(rot.pitch() / 2, rot_x_.x(), rot_x_.y());
    MathWorker::sincos(rot.yaw() / 2, rot_y_.x(), rot_y_.y());
    MathWorker::sincos(rot.roll() / 2, rot_z_.x(), rot_z_.y());

    Controller::AttachCamera(camera);
}

void FirstPersonController::DetachCamera()
{
    Controller::DetachCamera();
}

void FirstPersonController::Move(float x, float y, float z)
{
    if (camera_)
    {
        float3 movement(x, y, z);
        movement *= moveScaler_;

        auto& camera_node = *camera_->BoundSceneNode();
        camera_node.TransformToWorld(MathWorker::translation(movement) * camera_node.TransformToParent());

        camera_->Dirty();
    }
}

void FirstPersonController::RotateRel(float yaw, float pitch, float roll)
{
    if (camera_)
    {
        pitch *= -rotationScaler_ / 2;
        yaw *= -rotationScaler_ / 2;
        roll *= -rotationScaler_ / 2;

        float2 delta_x, delta_y, delta_z;
        MathWorker::sincos(pitch, delta_x.x(), delta_x.y());
        MathWorker::sincos(yaw, delta_y.x(), delta_y.y());
        MathWorker::sincos(roll, delta_z.x(), delta_z.y());

        quater quat_x(rot_x_.x() * delta_x.y() + rot_x_.y() * delta_x.x(), 0, 0, rot_x_.y() * delta_x.y() - rot_x_.x() * delta_x.x());
        quater quat_y(0, rot_y_.x() * delta_y.y() + rot_y_.y() * delta_y.x(), 0, rot_y_.y() * delta_y.y() - rot_y_.x() * delta_y.x());
        quater quat_z(0, 0, rot_z_.x() * delta_z.y() + rot_z_.y() * delta_z.x(), rot_z_.y() * delta_z.y() - rot_z_.x() * delta_z.x());

        rot_x_ = float2(quat_x.x(), quat_x.w());
        rot_y_ = float2(quat_y.y(), quat_y.w());
        rot_z_ = float2(quat_z.z(), quat_z.w());

        inv_rot_ = MathWorker::inverse(quat_y * quat_x * quat_z);
        float3 view_vec = MathWorker::transform_quat(float3(0, 0, 1), inv_rot_);
        float3 up_vec = MathWorker::transform_quat(float3(0, 1, 0), inv_rot_);

        auto& camera_node = *camera_->BoundSceneNode();
        camera_node.TransformToWorld(
            MathWorker::inverse(MathWorker::LookAtLH(camera_->EyePos(), camera_->EyePos() + view_vec * camera_->LookAtDist(), up_vec)));

        camera_->Dirty();
    }
}

void FirstPersonController::RotateAbs(const quater& quat)
{
    if (camera_)
    {
        float yaw, pitch, roll;
        MathWorker::ToYawPitchRoll(yaw, pitch, roll, quat);

        MathWorker::sincos(pitch / 2, rot_x_.x(), rot_x_.y());
        MathWorker::sincos(yaw / 2, rot_y_.x(), rot_y_.y());
        MathWorker::sincos(roll / 2, rot_z_.x(), rot_z_.y());

        inv_rot_ = MathWorker::inverse(quat);
        float3 view_vec = MathWorker::transform_quat(float3(0, 0, 1), inv_rot_);
        float3 up_vec = MathWorker::transform_quat(float3(0, 1, 0), inv_rot_);

        auto& camera_node = *camera_->BoundSceneNode();
        camera_node.TransformToWorld(
            MathWorker::inverse(MathWorker::LookAtLH(camera_->EyePos(), camera_->EyePos() + view_vec * camera_->LookAtDist(), up_vec)));
        camera_->Dirty();
    }
}
}