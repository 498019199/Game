#include <common/Util.h>
#include <render/Viewport.h>

namespace RenderWorker
{
Viewport::Viewport()
    :cameras_(1, CommonWorker::MakeSharedPtr<RenderWorker::Camera>())
{
    
}

Viewport::Viewport(int left, int top, int width, int height)
    :left_(left), top_(top), width_(width), height_(height),
        cameras_(1, CommonWorker::MakeSharedPtr<RenderWorker::Camera>())
{
    
}

uint32_t Viewport::NumCameras() const
{
	return static_cast<uint32_t>(cameras_.size());
}

void Viewport::NumCameras(uint32_t num)
{
    cameras_.resize(num);
}

void Viewport::Camera(const CameraPtr& camera)
{
    return this->Camera(0, camera);
}

const CameraPtr& Viewport::Camera() const
{
    return this->Camera(0);
}

void Viewport::Camera(uint32_t index, const CameraPtr& camera)
{
    cameras_[index] = camera;
}

const CameraPtr& Viewport::Camera(uint32_t index) const
{
    return cameras_[index];
}
}