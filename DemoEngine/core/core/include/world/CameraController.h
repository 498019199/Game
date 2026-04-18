#include <render/Camera.h>
#include <base/ZEngine.h>
#include <base/Input.h>

namespace RenderWorker
{
class ZENGINE_CORE_API CameraController
{
public:
    CameraController();

    void Scalers(float rotationScaler, float moveScaler);
    
    virtual void AttachCamera(Camera& camera);
    virtual void DetachCamera();

protected:
    float		rotationScaler_;	// Scaler for rotation
    float		moveScaler_;		// Scaler for movement
    Camera*     camera_;
};



using ControllerPtr = std::shared_ptr<CameraController>;

class ZENGINE_CORE_API FirstPersonController: public CameraController
{
public:
    FirstPersonController();
    ~FirstPersonController() noexcept;

    virtual void AttachCamera(Camera& camera) override;
    virtual void DetachCamera() override;

    void Move(float x, float y, float z);
    void RotateRel(float yaw, float pitch, float roll);
    void RotateAbs(const quater& quat);
private:
    float2		rot_x_;
    float2		rot_y_;
    float2		rot_z_;

    quater      inv_rot_;
};



class ZENGINE_CORE_API TrackballCameraController final : public CameraController
{
public:
    explicit TrackballCameraController(bool use_input_engine = true,
        uint32_t rotate_button = MB_Left, uint32_t zoom_button = MB_Right,
        uint32_t move_button = MB_Middle);

    virtual void AttachCamera(Camera& camera) override;

    void Move(float offset_x, float offset_y);
    void Rotate(float offset_x, float offset_y);
    void Zoom(float offset_x, float offset_y);

private:
    bool reverse_target_;
    float3 target_;
    float3 right_;
    uint32_t move_button_;
    uint32_t rotate_button_;
    uint32_t zoom_button_;

    enum
    {
        Turn,
        ZoomInOut
    };

private:
    void InputHandler(InputEngine const & sender, InputAction const & action);
};
}