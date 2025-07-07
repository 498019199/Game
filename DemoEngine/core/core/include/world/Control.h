


#include <Render/Camera.h>

namespace RenderWorker
{
class Controller
{
public:
    Controller();

    void Scalers(float rotationScaler, float moveScaler);
    
    virtual void AttachCamera(const CameraPtr& camera);
    virtual void DetachCamera();

    virtual void Move(float x, float y, float z) = 0;
    virtual void RotateRel(float yaw, float pitch, float roll) = 0;
    virtual void RotateAbs(const quater& quat) = 0;
protected:
    float		rotationScaler_;	// Scaler for rotation
    float		moveScaler_;		// Scaler for movement
    CameraPtr   camera_;
};



using ControllerPtr = std::shared_ptr<Controller>;

class FirstPersonController: public Controller
{
public:
    FirstPersonController();
    ~FirstPersonController() noexcept;

    virtual void AttachCamera(const CameraPtr& camera) override;
    virtual void DetachCamera() override;

    virtual void Move(float x, float y, float z) override;
    virtual void RotateRel(float yaw, float pitch, float roll) override;
    virtual void RotateAbs(const quater& quat) override;
private:
    float2		rot_x_;
    float2		rot_y_;
    float2		rot_z_;

    quater      inv_rot_;
};


}