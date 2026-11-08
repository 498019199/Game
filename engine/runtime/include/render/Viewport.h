#pragma once
#include <render/Camera.h>
// 视口
namespace RenderWorker
{
class ZENGINE_CORE_API Viewport final
{
    ZENGINE_NONCOPYABLE(Viewport);
public:
    Viewport();
    Viewport(int left, int top, int width, int height);

    void Left(int left)
    {
        left_ = left;
    }
    int Left() const
    {
        return left_;
    }
    void Top(int top)
    {
        top_ = top;
    }
    int Top() const
    {
        return top_;
    }
    void Width(int width)
    {
        width_ = width;
    }
    int Width() const
    {
        return width_;
    }
    void Height(int height)
    {
        height_ = height;
    }
    int Height() const
    {
        return height_;
    }

    uint32_t NumCameras() const;
    void NumCameras(uint32_t num);
    void Camera(const CameraPtr& camera);
    const CameraPtr& Camera() const;
    void Camera(uint32_t index, const CameraPtr& camera);
    const CameraPtr& Camera(uint32_t index) const;

private:
    int left_;
    int top_;
    int width_;
    int height_;

    std::vector<CameraPtr> cameras_;
};

using ViewportPtr = std::shared_ptr<Viewport>;
}