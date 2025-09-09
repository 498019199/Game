#pragma once
#include <common/common.h>
#include <render/RenderLayout.h>

#include <array>
#include <memory>

namespace RenderWorker
{
class RenderEffect;
class RenderPass;
class RenderTechnique;

struct ShaderDesc
{
    std::string profile;
    std::string func_name;
    uint64_t macros_hash;
    uint32_t tech_pass_type;

    ShaderDesc()
        : macros_hash(0), tech_pass_type(0xFFFFFFFF)
    {
    }

    // 定义D3D11_SO_DECLARATION_ENTRY，用于流输出（Stream Output）声明的结构体
    struct StreamOutputDecl
    {
        VertexElementUsage usage;   // 语义名
        uint8_t usage_index;        // 语义索引
        uint8_t start_component;    // 从第几个分量(xyzw)开始,只能取0-3
        uint8_t component_count;    // 分量的输出数目，只能取1-4
        uint8_t slot;               // 输出槽索引，只能取0-3

        friend bool operator==(const StreamOutputDecl& lhs, const StreamOutputDecl& rhs) noexcept;
        friend bool operator!=(const StreamOutputDecl& lhs, const StreamOutputDecl& rhs) noexcept;
    };
    static_assert(sizeof(StreamOutputDecl) == 8);

    std::vector<StreamOutputDecl> so_decl;
    friend bool operator==(const ShaderDesc& lhs, const ShaderDesc& rhs) noexcept;
    friend bool operator!=(const ShaderDesc& lhs, const ShaderDesc& rhs) noexcept;
};


enum class ShaderStage
{
    Vertex,
    Pixel,
    Geometry,
    Compute,
    Hull,
    Domain,

    NumStages,
};
uint32_t constexpr ShaderStageNum = std::to_underlying(ShaderStage::NumStages);

class ShaderStageObject
{
public:
    explicit ShaderStageObject(ShaderStage stage) noexcept;
    virtual ~ShaderStageObject() noexcept;

	virtual void CompileShader(const RenderEffect& effect, const RenderTechnique& tech, const RenderPass& pass,
			const std::array<uint32_t, ShaderStageNum>& shader_desc_ids) = 0;

    virtual void CreateHwShader(const RenderEffect& effect, const std::array<uint32_t, ShaderStageNum>& shader_desc_ids) = 0;

    bool Validate() const noexcept
    {
        return is_validate_;
    }

    // Pixel shader only
    virtual bool HasDiscard() const noexcept
    {
        return false;
    }

    bool HWResourceReady() const noexcept
    {
        return hw_res_ready_;
    }
protected:
    virtual void StageSpecificCreateHwShader(
        [[maybe_unused]] const RenderEffect& effect, [[maybe_unused]] const std::array<uint32_t, ShaderStageNum>& shader_desc_ids)
    {
    }

    static std::vector<uint8_t> CompileToDXBC(ShaderStage stage, RenderEffect const& effect, RenderTechnique const& tech,
        RenderPass const& pass, std::vector<std::pair<char const*, char const*>> const& api_special_macros, char const* func_name,
        char const* shader_profile, uint32_t flags, void** reflector, bool strip);

    virtual std::string_view GetShaderProfile(RenderEffect const& effect, uint32_t shader_desc_id) const = 0;
protected:
    const ShaderStage stage_;

    bool is_validate_ = false;
    bool hw_res_ready_ = false;
};

using ShaderStageObjectPtr = std::shared_ptr<ShaderStageObject>;

class ShaderObject;
struct Immutable;
using ShaderObjectPtr = std::shared_ptr<ShaderObject>;
class ShaderObject
{ 
public:
    ShaderObject();
    virtual ~ShaderObject() noexcept;

    void AttachStage(ShaderStage stage, const ShaderStageObjectPtr&  shader_stage);

    void LinkShaders(RenderEffect& effect);

    const ShaderStageObjectPtr&  Stage(ShaderStage stage) const noexcept;
    
    virtual void Bind(const RenderEffect& effect) = 0;
    virtual void Unbind() = 0;

    bool Validate() const noexcept
    {
        return immutable_->is_validate_;
    }
    bool HWResourceReady() const noexcept
    {
        return hw_res_ready_;
    }
private:
    virtual void DoLinkShaders(RenderEffect& effect) = 0;

protected:
    struct Immutable
    {
        std::array<ShaderStageObjectPtr, ShaderStageNum> shader_stages_;
        bool is_validate_;
    };

    explicit ShaderObject(std::shared_ptr<Immutable> immutable) noexcept;

protected:
    const std::shared_ptr<Immutable> immutable_;
    bool shader_stages_dirty_ = true;
    bool hw_res_ready_ = false;
};

}