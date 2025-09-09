#pragma once
#include <common/XMLDom.h>

#include <render/GraphicsBuffer.h>
#include <render/ShaderObject.h>
#include <render/Light.h>
#include <render/RenderStateObject.h>
#include <render/Texture.h>
#include <render/RenderView.h>

#include <common/common.h>
namespace RenderWorker
{
using namespace CommonWorker;

class RenderTechnique;
class RenderEffectStructType;
class RenderEffect;
using RenderEffectPtr = std::shared_ptr<RenderEffect>;
class RenderEffectConstantBuffer;
using RenderEffectConstantBufferPtr = std::shared_ptr<RenderEffectConstantBuffer>;
class RenderPass;
using RenderPassPtr = std::shared_ptr<RenderPass>;

enum RenderEffectDataType
{
    REDT_bool = 0,
    REDT_string,
    REDT_texture1D,
    REDT_texture2D,
    REDT_texture2DMS,
    REDT_texture3D,
    REDT_textureCUBE,
    REDT_texture1DArray,
    REDT_texture2DArray,
    REDT_texture2DMSArray,
    REDT_texture3DArray,
    REDT_textureCUBEArray,
    REDT_sampler,
    REDT_shader,
    REDT_uint,
    REDT_uint2,
    REDT_uint3,
    REDT_uint4,
    REDT_int,
    REDT_int2,
    REDT_int3,
    REDT_int4,
    REDT_float,
    REDT_float2,
    REDT_float2x2,
    REDT_float2x3,
    REDT_float2x4,
    REDT_float3,
    REDT_float3x2,
    REDT_float3x3,
    REDT_float3x4,
    REDT_float4,
    REDT_float4x2,
    REDT_float4x3,
    REDT_float4x4,
    REDT_buffer,
    REDT_structured_buffer,
    REDT_byte_address_buffer,
    REDT_rw_buffer,
    REDT_rw_structured_buffer,
    REDT_rw_texture1D,
    REDT_rw_texture2D,
    REDT_rw_texture3D,
    REDT_rw_texture1DArray,
    REDT_rw_texture2DArray,
    REDT_rw_byte_address_buffer,
    REDT_append_structured_buffer,
    REDT_consume_structured_buffer,
    REDT_rasterizer_ordered_buffer,
    REDT_rasterizer_ordered_byte_address_buffer,
    REDT_rasterizer_ordered_structured_buffer,
    REDT_rasterizer_ordered_texture1D,
    REDT_rasterizer_ordered_texture1DArray,
    REDT_rasterizer_ordered_texture2D,
    REDT_rasterizer_ordered_texture2DArray,
    REDT_rasterizer_ordered_texture3D,
    REDT_struct,

    REDT_count
};
static_assert(REDT_count < 256);

class RenderVariable
{
public:
    RenderVariable() noexcept;
    RenderVariable(RenderVariable const& rhs) = delete;
    virtual ~RenderVariable() noexcept;

    virtual RenderEffectStructType* StructType() const noexcept
    {
        return nullptr;
    }

    virtual std::unique_ptr<RenderVariable> Clone() = 0;

    virtual RenderVariable& operator=(bool const & value);
    virtual RenderVariable& operator=(uint32_t const & value);
    virtual RenderVariable& operator=(int32_t const & value);
    virtual RenderVariable& operator=(float const & value);
    virtual RenderVariable& operator=(uint2 const & value);
    virtual RenderVariable& operator=(uint3 const & value);
    virtual RenderVariable& operator=(uint4 const & value);
    virtual RenderVariable& operator=(int2 const & value);
    virtual RenderVariable& operator=(int3 const & value);
    virtual RenderVariable& operator=(int4 const & value);
    virtual RenderVariable& operator=(float2 const & value);
    virtual RenderVariable& operator=(float3 const & value);
    virtual RenderVariable& operator=(float4 const & value);
    virtual RenderVariable& operator=(float4x4 const & value);
    virtual RenderVariable& operator=(TexturePtr const & value);
    virtual RenderVariable& operator=(ShaderResourceViewPtr const & value);
    //virtual RenderVariable& operator=(UnorderedAccessViewPtr const & value);
    virtual RenderVariable& operator=(SamplerStateObjectPtr const & value);
    virtual RenderVariable& operator=(std::string const & value);
    virtual RenderVariable& operator=(std::string_view value);
    virtual RenderVariable& operator=(ShaderDesc const & value);
    virtual RenderVariable& operator=(std::vector<bool> const & value);
    virtual RenderVariable& operator=(std::vector<uint32_t> const & value);
    virtual RenderVariable& operator=(std::vector<int32_t> const & value);
    virtual RenderVariable& operator=(std::vector<float> const & value);
    virtual RenderVariable& operator=(std::vector<uint2> const & value);
    virtual RenderVariable& operator=(std::vector<uint3> const & value);
    virtual RenderVariable& operator=(std::vector<uint4> const & value);
    virtual RenderVariable& operator=(std::vector<int2> const & value);
    virtual RenderVariable& operator=(std::vector<int3> const & value);
    virtual RenderVariable& operator=(std::vector<int4> const & value);
    virtual RenderVariable& operator=(std::vector<float2> const & value);
    virtual RenderVariable& operator=(std::vector<float3> const & value);
    virtual RenderVariable& operator=(std::vector<float4> const & value);
    virtual RenderVariable& operator=(std::vector<float4x4> const & value);
    virtual RenderVariable& operator=(std::span<bool const> value);
    virtual RenderVariable& operator=(std::span<uint32_t const> value);
    virtual RenderVariable& operator=(std::span<int32_t const> value);
    virtual RenderVariable& operator=(std::span<float const> value);
    virtual RenderVariable& operator=(std::span<uint2 const> value);
    virtual RenderVariable& operator=(std::span<uint3 const> value);
    virtual RenderVariable& operator=(std::span<uint4 const> value);
    virtual RenderVariable& operator=(std::span<int2 const> value);
    virtual RenderVariable& operator=(std::span<int3 const> value);
    virtual RenderVariable& operator=(std::span<int4 const> value);
    virtual RenderVariable& operator=(std::span<float2 const> value);
    virtual RenderVariable& operator=(std::span<float3 const> value);
    virtual RenderVariable& operator=(std::span<float4 const> value);
    virtual RenderVariable& operator=(std::span<float4x4 const> value);

    // For struct
    virtual RenderVariable& operator=(std::vector<uint8_t> const& value);
    virtual RenderVariable& operator=(std::span<uint8_t const> value);

    virtual void Value(bool& val) const;
    virtual void Value(uint32_t& val) const;
    virtual void Value(int32_t& val) const;
    virtual void Value(float& val) const;
    virtual void Value(uint2& val) const;
    virtual void Value(uint3& val) const;
    virtual void Value(uint4& val) const;
    virtual void Value(int2& val) const;
    virtual void Value(int3& val) const;
    virtual void Value(int4& val) const;
    virtual void Value(float2& val) const;
    virtual void Value(float3& val) const;
    virtual void Value(float4& val) const;
    virtual void Value(float4x4& val) const;
    virtual void Value(TexturePtr& val) const;
    virtual void Value(ShaderResourceViewPtr& val) const;
    //virtual void Value(UnorderedAccessViewPtr& val) const;
    virtual void Value(SamplerStateObjectPtr& val) const;
    virtual void Value(std::string& val) const;
    virtual void Value(std::string_view& val) const;
    virtual void Value(ShaderDesc& val) const;
    virtual void Value(std::vector<bool>& val) const;
    virtual void Value(std::vector<uint32_t>& val) const;
    virtual void Value(std::vector<int32_t>& val) const;
    virtual void Value(std::vector<float>& val) const;
    virtual void Value(std::vector<uint2>& val) const;
    virtual void Value(std::vector<uint3>& val) const;
    virtual void Value(std::vector<uint4>& val) const;
    virtual void Value(std::vector<int2>& val) const;
    virtual void Value(std::vector<int3>& val) const;
    virtual void Value(std::vector<int4>& val) const;
    virtual void Value(std::vector<float2>& val) const;
    virtual void Value(std::vector<float3>& val) const;
    virtual void Value(std::vector<float4>& val) const;
    virtual void Value(std::vector<float4x4>& val) const;

    // For struct
    virtual void Value(std::vector<uint8_t>& val) const;

    virtual void BindToCBuffer(RenderEffect const& effect, uint32_t cbuff_index, uint32_t offset, uint32_t stride);
    virtual void RebindToCBuffer(RenderEffect const& effect, uint32_t cbuff_index);
    virtual bool InCBuffer() const noexcept
    {
        return false;
    }
    virtual RenderEffectConstantBuffer* CBuffer() const
    {
        return nullptr;
    }
    virtual uint32_t CBufferIndex() const
    {
        return 0;
    }
    virtual uint32_t CBufferOffset() const
    {
        return 0;
    }
    virtual uint32_t Stride() const
    {
        return 0;
    }

protected:
    RenderVariable& operator=(RenderVariable const& rhs);

protected:
    struct CBufferDesc
    {
        RenderEffect const* effect;
        uint32_t cbuff_index;
        uint32_t offset;
        uint32_t stride;
    };
};

class RenderShaderFragment final
{
public:
#if ZENGINE_IS_DEV_PLATFORM
    void Load(XMLNode const& node);
#endif

    void StreamIn(ResIdentifier& res);
#if ZENGINE_IS_DEV_PLATFORM
    void StreamOut(std::ostream& os) const;
#endif

    ShaderStage Stage() const noexcept
    {
        return stage_;
    }

    // ShaderModel Version() const noexcept
    // {
    //     return ver_;
    // }

    std::string const& str() const noexcept
    {
        return str_;
    }

private:
    ShaderStage stage_;
    //ShaderModel ver_;
    std::string str_;
};

class RenderEffectStructType final
{
public:
    RenderEffectStructType();
    RenderEffectStructType(RenderEffectStructType&& rhs) noexcept;
    RenderEffectStructType& operator=(RenderEffectStructType&& rhs) noexcept;

#if ZENGINE_IS_DEV_PLATFORM
    void Load(RenderEffect const& effect, XMLNode const& node);
#endif

    void StreamIn(ResIdentifier& res);
#if ZENGINE_IS_DEV_PLATFORM
	void StreamOut(std::ostream& os) const;
#endif

    std::string const& Name() const noexcept
    {
        return name_;
    }
    size_t NameHash() const noexcept
    {
        return name_hash_;
    }

    uint32_t NumMembers() const noexcept;
    RenderEffectDataType MemberType(uint32_t index) const noexcept;
    std::string const& MemberTypeName(uint32_t index) const noexcept;
    std::string const& MemberName(uint32_t index) const noexcept;
    std::string const* MemberArraySize(uint32_t index) const noexcept;

private:
    std::string name_;
    size_t name_hash_;

    struct StrcutMemberType
    {
        RenderEffectDataType type;
        std::string type_name;
        std::string name;
        std::unique_ptr<std::string> array_size;
    };
    std::vector<StrcutMemberType> members_;
};


class RenderEffectConstantBuffer final
{
public:
    explicit RenderEffectConstantBuffer(RenderEffect& effect);

#if ZENGINE_IS_DEV_PLATFORM
    void Load(std::string const & name);
#endif

    void StreamIn(ResIdentifier& res);
#if ZENGINE_IS_DEV_PLATFORM
	void StreamOut(std::ostream& os) const;
#endif

    RenderEffect& OwnerEffect() noexcept
    {
        return effect_;
    }

    std::string const& Name() const noexcept
    {
        return immutable_->name;
    }
    size_t NameHash() const noexcept
    {
        return immutable_->name_hash;
    }

    void AddParameter(uint32_t index);

    uint32_t NumParameters() const noexcept
    {
        return param_indices_ ? static_cast<uint32_t>(param_indices_->size()) : 0;
    }
    uint32_t ParameterIndex(uint32_t index) const noexcept;

    void Resize(uint32_t size);
    uint32_t Size() const noexcept
    {
        return static_cast<uint32_t>(buff_.size());
    }

    template <typename T>
    T const* VariableInBuff(uint32_t offset) const noexcept
    {
        union Raw2T
        {
            uint8_t const * raw;
            T const * t;
        } r2t;
        r2t.raw = &buff_[offset];
        return r2t.t;
    }
    template <typename T>
    T* VariableInBuff(uint32_t offset) noexcept
    {
        union Raw2T
        {
            uint8_t* raw;
            T* t;
        } r2t;
        r2t.raw = &buff_[offset];
        return r2t.t;
    }

    void Dirty(bool dirty) noexcept
    {
        dirty_ = dirty;
    }
    bool Dirty() const noexcept
    {
        return dirty_;
    }

    void Update();
    GraphicsBufferPtr const& HWBuff() const noexcept
    {
        return hw_buff_;
    }
    void BindHWBuff(GraphicsBufferPtr const & buff);

private:
    void RebindParameters(RenderEffectConstantBuffer& dst_cbuffer, RenderEffect& dst_effect);

private:
    RenderEffect& effect_;

    struct Immutable final
    {
        Immutable();

        std::string name;
        size_t name_hash;
    };

    std::shared_ptr<Immutable> immutable_;
    std::shared_ptr<std::vector<uint32_t>> param_indices_;

    GraphicsBufferPtr hw_buff_;
    std::vector<uint8_t> buff_;
    bool dirty_ = true;
};

class RenderEffectParameter final
{
public:
    RenderEffectParameter();
    RenderEffectParameter(RenderEffectParameter&& rhs) noexcept;
    RenderEffectParameter& operator=(RenderEffectParameter && rhs) noexcept;

#if ZENGINE_IS_DEV_PLATFORM
    void Load(RenderEffect const& effect, XMLNode const& node);
#endif

    void StreamIn(RenderEffect const& effect, ResIdentifier& res);
#if ZENGINE_IS_DEV_PLATFORM
	void StreamOut(std::ostream& os) const;
#endif

    RenderEffectDataType Type() const noexcept
    {
        return immutable_->type;
    }

    RenderEffectStructType* StructType() const noexcept
    {
        return var_->StructType();
    }

    RenderVariable const& Var() const noexcept;

    std::string const* ArraySize() const noexcept
    {
        return immutable_->array_size.get();
    }

    std::string const& Name() const noexcept
    {
        return immutable_->name;
    }
    size_t NameHash() const noexcept
    {
        return immutable_->name_hash;
    }
    bool HasSemantic() const noexcept
    {
        return !immutable_->semantic.empty();
    }
    std::string const & Semantic() const;
    size_t SemanticHash() const noexcept;

    template <typename T>
    RenderEffectParameter& operator=(T const & value)
    {
        *var_ = value;
        return *this;
    }

    template <typename T>
    void Value(T& val) const
    {
        var_->Value(val);
    }

    void BindToCBuffer(RenderEffect const& effect, uint32_t cbuff_index, uint32_t offset, uint32_t stride);
    void RebindToCBuffer(RenderEffect const& effect, uint32_t cbuff_index);
    RenderEffectConstantBuffer& CBuffer() const;
    bool InCBuffer() const noexcept
    {
        return var_->InCBuffer();
    }
    uint32_t CBufferIndex() const
    {
        return var_->CBufferIndex();
    }
    uint32_t CBufferOffset() const
    {
        return var_->CBufferOffset();
    }
    uint32_t Stride() const
    {
        return var_->Stride();
    }
    template <typename T>
    T const* MemoryInCBuff() const
    {
        return this->CBuffer().template VariableInBuff<T>(var_->CBufferOffset());
    }
    template <typename T>
    T* MemoryInCBuff()
    {
        return this->CBuffer().template VariableInBuff<T>(var_->CBufferOffset());
    }

private:
    struct Immutable final
    {
        Immutable();

        std::string name;
        size_t name_hash;
        std::string semantic;
        size_t semantic_hash;

        RenderEffectDataType type;
        std::unique_ptr<std::string> array_size;
    };

    std::shared_ptr<Immutable> immutable_;
    std::unique_ptr<RenderVariable> var_;
};
using RenderEffectConstantBufferPtr = std::shared_ptr<RenderEffectConstantBuffer>;

// 渲染效果
//////////////////////////////////////////////////////////////////////////////////
class RenderEffect final
{
public:
    RenderEffect();

    void Load(std::span<std::string const> names);
#if ZENGINE_IS_DEV_PLATFORM
    void CompileShaders();
#endif
    void CreateHwShaders();

    RenderEffectPtr Clone();
    void CloneInPlace(RenderEffect& dst_effect);
    void Reclone(RenderEffect& dst_effect);

    bool HWResourceReady() const noexcept;

    std::string const& ResName() const noexcept
    {
        return immutable_->res_name;
    }
    size_t ResNameHash() const noexcept
    {
        return immutable_->res_name_hash;
    }

    uint32_t NumParameters() const noexcept
    {
        return static_cast<uint32_t>(params_.size());
    }
    RenderEffectParameter* ParameterBySemantic(std::string_view semantic) noexcept;
    RenderEffectParameter const* ParameterBySemantic(std::string_view semantic) const noexcept;
    RenderEffectParameter* ParameterByName(std::string_view name) noexcept;
    RenderEffectParameter const* ParameterByName(std::string_view name) const noexcept;
    RenderEffectParameter* ParameterByIndex(uint32_t n) noexcept;
    RenderEffectParameter const* ParameterByIndex(uint32_t n) const noexcept;

    uint32_t NumCBuffers() const noexcept
    {
        return static_cast<uint32_t>(cbuffers_.size());
    }
    RenderEffectConstantBuffer* CBufferByName(std::string_view name) const noexcept;
    RenderEffectConstantBuffer* CBufferByIndex(uint32_t index) const noexcept;
    uint32_t FindCBuffer(std::string_view name) const noexcept;

    void BindCBufferByName(std::string_view name, RenderEffectConstantBufferPtr const& cbuff) noexcept;
    void BindCBufferByIndex(uint32_t index, RenderEffectConstantBufferPtr const& cbuff) noexcept;

    uint32_t NumStructTypes() const noexcept
    {
        return static_cast<uint32_t>(immutable_->struct_types.size());
    }
    RenderEffectStructType* StructTypeByName(std::string_view name) const noexcept;
    RenderEffectStructType* StructTypeByIndex(uint32_t index) const noexcept;

    uint32_t NumTechniques() const noexcept;
    RenderTechnique* TechniqueByName(std::string_view name) const noexcept;
    RenderTechnique* TechniqueByIndex(uint32_t n) const noexcept;

    uint32_t NumShaderFragments() const noexcept
    {
        return static_cast<uint32_t>(immutable_->shader_frags.size());
    }
    RenderShaderFragment const& ShaderFragmentByIndex(uint32_t n) const noexcept;

    uint32_t AddShaderDesc(ShaderDesc const & sd);
    ShaderDesc& GetShaderDesc(uint32_t id) noexcept;
    ShaderDesc const& GetShaderDesc(uint32_t id) const noexcept;

    uint32_t NumMacros() const noexcept
    {
        return static_cast<uint32_t>(immutable_->macros.size());
    }
    std::pair<std::string, std::string> const& MacroByIndex(uint32_t n) const noexcept;

    uint32_t AddShaderObject();
    ShaderObjectPtr const& ShaderObjectByIndex(uint32_t n) const noexcept;

#if ZENGINE_IS_DEV_PLATFORM
    void GenHLSLShaderText();
    const std::string& HLSLShaderText() const noexcept
    {
        return immutable_->hlsl_shader;
    }
#endif
    
private:
    bool StreamIn(ResIdentifier& source);
#if ZENGINE_IS_DEV_PLATFORM
    void StreamOut(std::ostream& os) const;
#endif

#if ZENGINE_IS_DEV_PLATFORM
    void PreprocessIncludes(XMLNode& root, std::vector<std::string>& include_names);
    void RecursiveIncludeNode(XMLNode const& root, std::vector<std::string>& include_names) const;

    XMLNode ResolveInheritTechNode(XMLNode& root, XMLNode const* tech_node);
    void ResolveOverrideTechs(XMLNode& root);

    void Load(XMLNode const& root);
#endif

private:
    struct Immutable final
    {
        Immutable();

        std::string res_name;
        size_t res_name_hash;
#if ZENGINE_IS_DEV_PLATFORM
        uint64_t timestamp;

        std::string kfx_name;
        bool need_compile;
#endif

        std::vector<RenderEffectStructType> struct_types;

        std::vector<RenderTechnique> techniques;

        std::vector<std::pair<std::string, std::string>> macros;
        std::vector<RenderShaderFragment> shader_frags;
#if ZENGINE_IS_DEV_PLATFORM
        std::string hlsl_shader;
#endif

        std::vector<ShaderDesc> shader_descs;

        //std::vector<RenderShaderGraphNode> shader_graph_nodes;
    };

    std::shared_ptr<Immutable> immutable_;

    std::vector<RenderEffectParameter> params_;
    std::vector<RenderEffectConstantBufferPtr> cbuffers_;
    std::vector<ShaderObjectPtr> shader_objs_;

    mutable bool hw_res_ready_ = false;
};


class RenderTechnique final
{
public:
    RenderTechnique();
    RenderTechnique(RenderTechnique&& rhs) noexcept;
    RenderTechnique& operator=(RenderTechnique&& rhs) noexcept;

#if ZENGINE_IS_DEV_PLATFORM
    void Load(RenderEffect& effect, XMLNode const& node, uint32_t tech_index);
    void CompileShaders(RenderEffect& effect, uint32_t tech_index);
#endif
    void CreateHwShaders(RenderEffect& effect, uint32_t tech_index);

    bool StreamIn(RenderEffect& effect, ResIdentifier& res, uint32_t tech_index);
#if ZENGINE_IS_DEV_PLATFORM
    void StreamOut(RenderEffect const & effect, std::ostream& os, uint32_t tech_index) const;
#endif
    
    std::string const & Name() const noexcept
    {
        return name_;
    }
    size_t NameHash() const noexcept
    {
        return name_hash_;
    }

    uint32_t NumMacros() const noexcept
    {
        return macros_ ? static_cast<uint32_t>(macros_->size()) : 0;
    }
    std::pair<std::string, std::string> const& MacroByIndex(uint32_t n) const noexcept;

    uint32_t NumPasses() const noexcept
    {
        return static_cast<uint32_t>(passes_.size());
    }
    RenderPass const& Pass(uint32_t n) const noexcept;

    bool Validate() const noexcept
    {
        return is_validate_;
    }

    bool HWResourceReady(RenderEffect const& effect) const noexcept;

    float Weight() const noexcept
    {
        return weight_;
    }
    bool Transparent() const noexcept
    {
        return transparent_;
    }
    bool HasDiscard() const noexcept
    {
        return has_discard_;
    }
    bool HasTessellation() const noexcept
    {
        return has_tessellation_;
    }

private:
    std::string name_;
    size_t name_hash_;

    std::vector<RenderPassPtr> passes_;
    std::shared_ptr<std::vector<std::pair<std::string, std::string>>> macros_;

    float weight_;
    bool transparent_;

    bool is_validate_;
    bool has_discard_;
    bool has_tessellation_;
};

class RenderPass final
{
public:
    RenderPass();

#if ZENGINE_IS_DEV_PLATFORM
    void Load(RenderEffect& effect, XMLNode const& node, uint32_t tech_index, uint32_t pass_index, RenderPass const* inherit_pass);
    void Load(RenderEffect& effect, uint32_t tech_index, uint32_t pass_index, RenderPass const* inherit_pass);
    void CompileShaders(RenderEffect& effect, uint32_t tech_index, uint32_t pass_index);
#endif
    void CreateHwShaders(RenderEffect& effect, uint32_t tech_index, uint32_t pass_index);

    bool StreamIn(RenderEffect& effect, ResIdentifier& res, uint32_t tech_index, uint32_t pass_index);
#if ZENGINE_IS_DEV_PLATFORM
    void StreamOut(RenderEffect const & effect, std::ostream& os, uint32_t tech_index, uint32_t pass_index) const;
#endif
    std::string const & Name() const noexcept
    {
        return name_;
    }
    size_t NameHash() const noexcept
    {
        return name_hash_;
    }

    void Bind(RenderEffect const & effect) const;
    void Unbind(RenderEffect const & effect) const;

    bool Validate() const noexcept
    {
        return is_validate_;
    }

    RenderStateObjectPtr const& GetRenderStateObject() const noexcept
    {
        return render_state_obj_;
    }
    ShaderObjectPtr const& GetShaderObject(RenderEffect const& effect) const noexcept
    {
        return effect.ShaderObjectByIndex(shader_obj_index_);
    }

    uint32_t NumMacros() const noexcept
    {
        return macros_ ? static_cast<uint32_t>(macros_->size()) : 0;
    }
    std::pair<std::string, std::string> const& MacroByIndex(uint32_t n) const noexcept;

private:
    std::string name_;
    size_t name_hash_;
    std::shared_ptr<std::vector<std::pair<std::string, std::string>>> macros_;
    std::array<uint32_t, ShaderStageNum> shader_desc_ids_;

    RenderStateObjectPtr render_state_obj_;
    uint32_t shader_obj_index_;

    bool is_validate_;
};

RenderEffectPtr SyncLoadRenderEffect(std::string_view effect_names);
}