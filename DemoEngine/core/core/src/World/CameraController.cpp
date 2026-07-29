#include <world/CameraController.h>
#include <world/SceneNode.h>
#include <base/InputFactory.h>
#include <base/UIManager.h>

#include <algorithm>

namespace RenderWorker
{
namespace
{
    constexpr float MinTrackballDistance = 0.01f;
    constexpr float MouseWheelZoomScale = 0.01f;
}

CameraController::CameraController()
    :rotationScaler_(0.05f), moveScaler_(1), camera_(nullptr)
{
    
}

void CameraController::Scalers(float rotationScaler, float moveScaler)
{
    rotationScaler_ = rotationScaler;
    moveScaler_ = moveScaler;
}

void CameraController::InputEnabled(bool enabled)
{
    input_enabled_ = enabled;
}

bool CameraController::InputEnabled() const
{
    return input_enabled_;
}

void CameraController::AttachCamera(Camera&  camera)
{
    camera_ = &camera;
}

void CameraController::DetachCamera()
{
    camera_ = nullptr;
}

    


FirstPersonController::FirstPersonController( bool use_input_engine )
    : left_button_down_(false)
{
    if (use_input_engine)
    {
        InputActionDefine actions[] =
        {
            InputActionDefine(TurnLeftRight, MS_X),
            InputActionDefine(TurnUpDown, MS_Y),
            InputActionDefine(RollLeft, KS_Q),
            InputActionDefine(RollRight, KS_E),
            InputActionDefine(Forward, KS_W),
            InputActionDefine(Backward, KS_S),
            InputActionDefine(MoveLeft, KS_A),
            InputActionDefine(MoveRight, KS_D),
            InputActionDefine(Forward, KS_UpArrow),
            InputActionDefine(Backward, KS_DownArrow),
            InputActionDefine(MoveLeft, KS_LeftArrow),
            InputActionDefine(MoveRight, KS_RightArrow),
            InputActionDefine(Turn, TS_Pan),
            InputActionDefine(Turn, SS_OrientationQuat)
        };

        InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
        InputActionMap actionMap;
        actionMap.AddActions(actions, actions + std::size(actions));

        action_handler_t input_handler = MakeSharedPtr<input_signal>();
        input_handler->Connect(
            [this](InputEngine const & ie, InputAction const & action)
            {
                this->InputHandler(ie, action);
            });
        inputEngine.ActionMap(actionMap, input_handler);
    }
}

FirstPersonController::~FirstPersonController() noexcept  = default;

void FirstPersonController::AttachCamera(Camera& camera)
{
    float3 scale;
    float3 translation;
    quater quat;
    MathWorker::decompose(scale, quat, translation, camera.ViewMatrix());

    rotator rot = MathWorker::ToRotator(quat);

    MathWorker::sincos(rot.pitch() / 2, rot_x_.x(), rot_x_.y());
    MathWorker::sincos(rot.yaw() / 2, rot_y_.x(), rot_y_.y());
    MathWorker::sincos(rot.roll() / 2, rot_z_.x(), rot_z_.y());

    CameraController::AttachCamera(camera);
}

void FirstPersonController::DetachCamera()
{
    CameraController::DetachCamera();
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
            MathWorker::inverse(MathWorker::look_at_lh(camera_->EyePos(), camera_->EyePos() + view_vec * camera_->LookAtDist(), up_vec)));

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
            MathWorker::inverse(MathWorker::look_at_lh(camera_->EyePos(), camera_->EyePos() + view_vec * camera_->LookAtDist(), up_vec)));
        camera_->Dirty();
    }
}

void FirstPersonController::InputHandler(InputEngine const & ie, InputAction const & action)
{
    float elapsed_time = ie.ElapsedTime();
    // Mouse deltas arrive window-wide; the host decides where looking around is allowed.
    if (!input_enabled_ && (action.second->type == InputEngine::IDT_Mouse))
    {
        return;
    }

    if (camera_)
    {
        float const scaler = elapsed_time * 10;

        switch (action.first)
        {
        case TurnLeftRight:
            {
                InputMouseActionParamPtr param = checked_pointer_cast<InputMouseActionParam>(action.second);
                if (!left_button_down_ || (param->buttons_state & MB_Left))
                {
                    this->RotateRel(param->move_vec.x() * scaler, 0, 0);
                }
            }
            break;

        case TurnUpDown:
            {
                InputMouseActionParamPtr param = checked_pointer_cast<InputMouseActionParam>(action.second);
                if (!left_button_down_ || (param->buttons_state & MB_Left))
                {
                    this->RotateRel(0, param->move_vec.y() * scaler, 0);
                }
            }
            break;

        case Turn:
            switch (action.second->type)
            {
            case InputEngine::IDT_Touch:
                {
                    InputTouchActionParamPtr param = checked_pointer_cast<InputTouchActionParam>(action.second);
                    this->RotateRel(param->move_vec.x() * scaler, param->move_vec.y() * scaler, 0);
                }
                break;

            case InputEngine::IDT_Sensor:
                {
                    InputSensorActionParamPtr param = checked_pointer_cast<InputSensorActionParam>(action.second);
                    this->RotateAbs(param->orientation_quat);
                }
                break;

            default:
                ZENGINE_UNREACHABLE("Invalid input type");
            }
            break;

        case RollLeft:
            this->RotateRel(0, 0, -scaler);
            break;

        case RollRight:
            this->RotateRel(0, 0, scaler);
            break;

        case Forward:
            this->Move(0, 0, scaler);
            break;

        case Backward:
            this->Move(0, 0, -scaler);
            break;

        case MoveLeft:
            this->Move(-scaler, 0, 0);
            break;

        case MoveRight:
            this->Move(scaler, 0, 0);
            break;
        }
    }
}



TrackballCameraController::TrackballCameraController(bool use_input_engine, uint32_t rotate_button,
    uint32_t zoom_button, uint32_t move_button)
        : move_button_(move_button), rotate_button_(rotate_button), zoom_button_(zoom_button)
{
    if (use_input_engine)
    {
        InputActionDefine actions[] =
        {
            InputActionDefine(Turn, MS_X),
            InputActionDefine(Turn, MS_Y),
            InputActionDefine(ZoomInOut, MS_Z),
            InputActionDefine(Turn, TS_Pan),
            InputActionDefine(Turn, TS_Pan),
            InputActionDefine(ZoomInOut, TS_Zoom)
        };

        InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
        InputActionMap actionMap;
        actionMap.AddActions(actions, actions + std::size(actions));

        action_handler_t input_handler = MakeSharedPtr<input_signal>();
        input_handler->Connect(
            [this](InputEngine const & ie, InputAction const & action)
            {
                this->InputHandler(ie, action);
            });
        inputEngine.ActionMap(actionMap, input_handler);
    }
}

void TrackballCameraController::InputHandler(InputEngine const & /*ie*/, InputAction const & action)
{
    if (input_enabled_ && camera_ && !Context::Instance().UIManagerInstance().MouseOnUI() )
    {
        switch (action.first)
        {
        case Turn:
            switch (action.second->type)
            {
            case InputEngine::IDT_Mouse:
                {
                    InputMouseActionParamPtr param = checked_pointer_cast<InputMouseActionParam>(action.second);

                    float xd = static_cast<float>(param->move_vec.x());
                    float yd = static_cast<float>(param->move_vec.y());

                    if (param->buttons_state & rotate_button_)
                    {
                        this->Rotate(xd, yd);
                    }
                    else if (param->buttons_state & zoom_button_)
                    {
                        this->Zoom(xd, yd);
                    }
                    else if (param->buttons_state & move_button_)
                    {
                        this->Move(xd, yd);
                    }
                }
                break;

            case InputEngine::IDT_Touch:
                {
                    InputTouchActionParamPtr param = checked_pointer_cast<InputTouchActionParam>(action.second);

                    float xd = static_cast<float>(param->move_vec.x());
                    float yd = static_cast<float>(param->move_vec.y());

                    this->Rotate(xd, yd);
                }
                break;

            default:
                break;
            }
            break;

        case ZoomInOut:
            switch (action.second->type)
            {
            case InputEngine::IDT_Mouse:
                {
                    InputMouseActionParamPtr param = checked_pointer_cast<InputMouseActionParam>(action.second);
                    this->Zoom(static_cast<float>(param->wheel_delta) * MouseWheelZoomScale, 0);
                }
                break;

            case InputEngine::IDT_Touch:
                {
                    InputTouchActionParamPtr param = checked_pointer_cast<InputTouchActionParam>(action.second);
                    this->Zoom(param->zoom, 0);
                }
                break;

            default:
                break;
            }
            break;
        }
    }
}

void TrackballCameraController::AttachCamera(Camera& camera)
{
    CameraController::AttachCamera(camera);

    reverse_target_ = false;
    target_ = camera_->LookAt();
    right_ = camera_->RightVec();
}

void TrackballCameraController::Move(float offset_x, float offset_y)
{
    float3 offset = right_ * (-offset_x * moveScaler_ * 2);
    float3 pos = camera_->EyePos() + offset;
    target_ += offset;

    offset = camera_->UpVec() * (offset_y * moveScaler_ * 2);
    pos += offset;
    target_ += offset;

    camera_->LookAtDist(MathWorker::length(target_ - pos));
    auto& camera_node = *camera_->BoundSceneNode();
    camera_node.TransformToWorld(MathWorker::inverse(MathWorker::look_at_lh(pos, target_, camera_->UpVec())));

    camera_->Dirty();
}

void TrackballCameraController::Rotate(float offset_x, float offset_y)
{
    quater q = MathWorker::rotation_axis(right_, offset_y * rotationScaler_);
    float4x4 mat = MathWorker::transformation<float>(nullptr, nullptr, nullptr, &target_, &q, nullptr);
    float3 pos = MathWorker::transform_coord(camera_->EyePos(), mat);

    q = MathWorker::rotation_axis(float3(0.0f, MathWorker::sgn(camera_->UpVec().y()), 0.0f), offset_x * rotationScaler_);
    mat = MathWorker::transformation<float>(nullptr, nullptr, nullptr, &target_, &q, nullptr);
    pos = MathWorker::transform_coord(pos, mat);

    right_ = MathWorker::transform_quat(right_, q);

    float3 dir;
    if (reverse_target_)
    {
        dir = pos - target_;
    }
    else
    {
        dir = target_ - pos;
    }
    float dist = MathWorker::length(dir);
    dir /= dist;
    float3 up = MathWorker::cross(dir, right_);

    camera_->LookAtDist(dist);
    auto& camera_node = *camera_->BoundSceneNode();
    camera_node.TransformToWorld(MathWorker::inverse(MathWorker::look_at_lh(pos, pos + dir * dist, up)));

    camera_->Dirty();
}

void TrackballCameraController::Zoom(float offset_x, float offset_y)
{
    float3 offset = camera_->ForwardVec() * ((offset_x + offset_y) * moveScaler_ * 2);
    float3 pos = camera_->EyePos() + offset;

    if (MathWorker::dot(target_ - pos, camera_->ForwardVec()) <= 0)
    {
        reverse_target_ = true;
    }
    else
    {
        reverse_target_ = false;
    }

    auto& camera_node = *camera_->BoundSceneNode();
    camera_node.TransformToWorld(
        MathWorker::inverse(MathWorker::look_at_lh(pos, pos + camera_->ForwardVec() * camera_->LookAtDist(), camera_->UpVec())));

    camera_->Dirty();
}

}