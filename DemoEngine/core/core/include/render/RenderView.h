#pragma once
#include <base/ZEngine.h>
#include <render/Texture.h>
#include <render/FrameBuffer.h>
#include <render/GraphicsBuffer.h>

namespace RenderWorker
{
// 抽象SRV,描述着色器如何访问资源
class ZENGINE_CORE_API ShaderResourceView
{
    ZENGINE_NONCOPYABLE(ShaderResourceView);
public:
    ShaderResourceView();
    virtual ~ShaderResourceView() noexcept;

    ElementFormat Format() const
    {
        return pf_;
    }

    TexturePtr const & TextureResource() const
    {
        return tex_;
    }
    uint32_t FirstArrayIndex() const
    {
        return first_array_index_;
    }
    uint32_t ArraySize() const
    {
        return array_size_;
    }
    uint32_t FirstLevel() const
    {
        return first_level_;
    }
    uint32_t NumLevels() const
    {
        return num_levels_;
    }

    GraphicsBufferPtr const &BufferResource() const
    {
        return buff_;
    }
    uint32_t FirstElement() const
    {
        return first_elem_;
    }
    uint32_t NumElements() const
    {
        return num_elems_;
    }
protected:
    ElementFormat pf_;

    // For textures
    TexturePtr tex_;
    uint32_t first_array_index_;
    uint32_t array_size_;
    uint32_t first_level_;
    uint32_t num_levels_;

    // For graphics buffers
    GraphicsBufferPtr buff_;
    uint32_t first_elem_;
    uint32_t num_elems_;
};
using ShaderResourceViewPtr = std::shared_ptr<ShaderResourceView>;


// 抽象RTV,用于将资源（如纹理）指定为渲染目标，使 GPU 能够将渲染结果（如像素颜色）写入该资源。
class ZENGINE_CORE_API RenderTargetView
{
    ZENGINE_NONCOPYABLE(RenderTargetView);
public:
    RenderTargetView();
    virtual ~RenderTargetView() noexcept;

    uint32_t Width() const
    {
        return width_;
    }
    uint32_t Height() const
    {
        return height_;
    }
    ElementFormat Format() const
    {
        return pf_;
    }
    uint32_t SampleCount() const
    {
        return sample_count_;
    }
    uint32_t SampleQuality() const
    {
        return sample_quality_;
    }

    TexturePtr const & TextureResource() const
    {
        return tex_;
    }
    uint32_t FirstArrayIndex() const
    {
        return first_array_index_;
    }
    uint32_t ArraySize() const
    {
        return array_size_;
    }
    uint32_t Level() const
    {
        return level_;
    }
    uint32_t FirstSlice() const
    {
        return first_slice_;
    }
    uint32_t NumSlices() const
    {
        return num_slices_;
    }
    Texture::CubeFaces FirstFace() const
    {
        return first_face_;
    }
    uint32_t NumFaces() const
    {
        return num_faces_;
    }

    GraphicsBufferPtr const & BufferResource() const
    {
        return buff_;
    }
    uint32_t FirstElement() const
    {
        return first_elem_;
    }
    uint32_t NumElements() const
    {
        return num_elems_;
    }

    virtual void ClearColor(Color const & clr) = 0;

    virtual void Discard() = 0;

    virtual void OnAttached(FrameBuffer& fb, FrameBuffer::Attachment att) = 0;
    virtual void OnDetached(FrameBuffer& fb, FrameBuffer::Attachment att) = 0;

protected:
    uint32_t width_;
    uint32_t height_;
    ElementFormat pf_;
    uint32_t sample_count_;
    uint32_t sample_quality_;

    // For textures
    TexturePtr tex_;
    uint32_t first_array_index_;
    uint32_t array_size_;
    uint32_t level_;

    // For 3D textures
    uint32_t first_slice_;
    uint32_t num_slices_;

    // For cube textures
    Texture::CubeFaces first_face_;
    uint32_t num_faces_;

    // For buffers
    GraphicsBufferPtr buff_;
    uint32_t first_elem_;
    uint32_t num_elems_;
};
using RenderTargetViewPtr = std::shared_ptr<RenderTargetView>;

// 抽象DSV,用于将资源（通常是深度模板缓冲区）绑定到渲染管线，以支持深度测试（Depth Testing）和模板测试（Stencil Testing）
class ZENGINE_CORE_API DepthStencilView
{
    ZENGINE_NONCOPYABLE(DepthStencilView);
public:
    DepthStencilView();
    virtual ~DepthStencilView() noexcept;

    uint32_t Width() const
    {
        return width_;
    }
    uint32_t Height() const
    {
        return height_;
    }
    ElementFormat Format() const
    {
        return pf_;
    }
    uint32_t SampleCount() const
    {
        return sample_count_;
    }
    uint32_t SampleQuality() const
    {
        return sample_quality_;
    }

    TexturePtr const & TextureResource() const
    {
        return tex_;
    }
    uint32_t FirstArrayIndex() const
    {
        return first_array_index_;
    }
    uint32_t ArraySize() const
    {
        return array_size_;
    }
    uint32_t Level() const
    {
        return level_;
    }
    uint32_t FirstSlice() const
    {
        return first_slice_;
    }
    uint32_t NumSlices() const
    {
        return num_slices_;
    }
    Texture::CubeFaces FirstFace() const
    {
        return first_face_;
    }
    uint32_t NumFaces() const
    {
        return num_faces_;
    }

    virtual void ClearDepth(float depth) = 0;
    virtual void ClearStencil(int32_t stencil) = 0;
    virtual void ClearDepthStencil(float depth, int32_t stencil) = 0;

    virtual void Discard() = 0;

    virtual void OnAttached(FrameBuffer& fb) = 0;
    virtual void OnDetached(FrameBuffer& fb) = 0;

protected:
    uint32_t width_;
    uint32_t height_;
    ElementFormat pf_;
    uint32_t sample_count_;
    uint32_t sample_quality_;

    // For textures
    TexturePtr tex_;
    uint32_t first_array_index_;
    uint32_t array_size_;
    uint32_t level_;

    // For 3D textures
    uint32_t first_slice_;
    uint32_t num_slices_;

    // For cube textures
    Texture::CubeFaces first_face_;
    uint32_t num_faces_;
};
using DepthStencilViewPtr = std::shared_ptr<DepthStencilView>;


// 抽象UAV,用于无序访问资源的接口，允许着色器（尤其是计算着色器）以非顺序方式读写资源数据，支持随机访问和并发操作。
// 它是实现复杂并行计算（如粒子系统、物理模拟、后处理效果）的核心机制。
class ZENGINE_CORE_API UnorderedAccessView
{
    ZENGINE_NONCOPYABLE(UnorderedAccessView);
public:
    UnorderedAccessView();
    virtual ~UnorderedAccessView() noexcept;

    ElementFormat Format() const
    {
        return pf_;
    }

    void InitCount(uint32_t count)
    {
        init_count_ = count;
    }
    uint32_t InitCount() const
    {
        return init_count_;
    }

    TexturePtr const & TextureResource() const
    {
        return tex_;
    }
    uint32_t FirstArrayIndex() const
    {
        return first_array_index_;
    }
    uint32_t ArraySize() const
    {
        return array_size_;
    }
    uint32_t Level() const
    {
        return level_;
    }
    uint32_t FirstSlice() const
    {
        return first_slice_;
    }
    uint32_t NumSlices() const
    {
        return num_slices_;
    }
    Texture::CubeFaces FirstFace() const
    {
        return first_face_;
    }
    uint32_t NumFaces() const
    {
        return num_faces_;
    }

    GraphicsBufferPtr const & BufferResource() const
    {
        return buff_;
    }
    uint32_t FirstElement() const
    {
        return first_elem_;
    }
    uint32_t NumElements() const
    {
        return num_elems_;
    }

    virtual void Clear(float4 const & val) = 0;
    virtual void Clear(uint4 const & val) = 0;

    virtual void Discard() = 0;

    virtual void OnAttached(FrameBuffer& fb, uint32_t att) = 0;
    virtual void OnDetached(FrameBuffer& fb, uint32_t att) = 0;

protected:
    ElementFormat pf_;
    uint32_t init_count_ = 0;

    // For textures
    TexturePtr tex_;
    uint32_t first_array_index_;
    uint32_t array_size_;
    uint32_t level_;

    // For 3D textures
    uint32_t first_slice_;
    uint32_t num_slices_;

    // For cube textures
    Texture::CubeFaces first_face_;
    uint32_t num_faces_;

    // For buffers
    GraphicsBufferPtr buff_;
    uint32_t first_elem_;
    uint32_t num_elems_;
};

using UnorderedAccessViewPtr = std::shared_ptr<UnorderedAccessView>;
}