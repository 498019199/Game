#pragma once
#include <common/Util.h>
#include <common/common.h>
#include <render/Texture.h>
#include <render/GraphicsBuffer.h>

namespace RenderWorker
{
class ShaderResourceView
{
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
}