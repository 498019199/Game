#include "D3D11ShaderObject.h"
#include "D3D11RenderFactory.h"
#include "D3D11GraphicsBuffer.h"
#include "D3D11RenderEngine.h"
#include "D3D11RenderStateObject.h"
#include "D3D11RenderView.h"

#include <base/ZEngine.h>
#include <common/CustomizedStreamBuf.h>
#if ZENGINE_IS_DEV_PLATFORM
#include <d3dx11.h>
#include <d3dcompiler.h>
#endif


namespace
{
using namespace RenderWorker;

class D3D11ShaderParameterSrvUpdater final
{
public:
    D3D11ShaderParameterSrvUpdater(
        std::tuple<void*, uint32_t, uint32_t>& srvsrc, ID3D11ShaderResourceView*& srv, RenderEffectParameter const& param)
        : srvsrc_(srvsrc), srv_(srv), param_(param)
    {
    }

    void operator()()
    {
        ShaderResourceViewPtr srv;
        param_.Value(srv);
        if (srv)
        {
            if (srv->TextureResource())
            {
                srvsrc_ = std::make_tuple(srv->TextureResource().get(),
                    srv->FirstArrayIndex() * srv->TextureResource()->MipMapsNum() + srv->FirstLevel(),
                    srv->ArraySize() * srv->NumLevels());
            }
            else
            {
                COMMON_ASSERT(srv->BufferResource());
                srvsrc_ = std::make_tuple(srv->BufferResource().get(), 0, 1);
            }

            srv_ = checked_cast<D3D11ShaderResourceView&>(*srv).RetrieveD3DShaderResourceView();
        }
        else
        {
            std::get<0>(srvsrc_) = nullptr;
            srv_ = nullptr;
        }
    }

private:
    std::tuple<void*, uint32_t, uint32_t>& srvsrc_;
    ID3D11ShaderResourceView*& srv_;
    RenderEffectParameter const& param_;
};
}

namespace RenderWorker
{
using namespace CommonWorker;

D3D11ShaderStageObject::D3D11ShaderStageObject(ShaderStage stage)
    : ShaderStageObject(stage)
{
}

void D3D11ShaderStageObject::StreamIn(
    const RenderEffect& effect, const std::array<uint32_t, ShaderStageNum>& shader_desc_ids, ResIdentifier& res)
{
    uint32_t native_shader_block_len;
    res.read(&native_shader_block_len, sizeof(native_shader_block_len));
    native_shader_block_len = LE2Native(native_shader_block_len);

    is_validate_ = false;
    std::string_view const shader_profile = this->GetShaderProfile(effect, shader_desc_ids[std::to_underlying(stage_)]);
    if (native_shader_block_len >= 25 + shader_profile.size())
    {
        uint8_t len;
        res.read(reinterpret_cast<char*>(&len), sizeof(len));
        std::string& profile = shader_profile_;
        profile.resize(len);
        res.read(&profile[0], len);
        if (profile == shader_profile)
        {
            is_validate_ = true;

            uint32_t blob_size;
            res.read(reinterpret_cast<char*>(&blob_size), sizeof(blob_size));
            shader_code_.resize(blob_size);

            res.read(reinterpret_cast<char*>(shader_code_.data()), blob_size);

            uint16_t cb_desc_size;
            res.read(reinterpret_cast<char*>(&cb_desc_size), sizeof(cb_desc_size));
            cb_desc_size = LE2Native(cb_desc_size);
            shader_desc_.cb_desc.resize(cb_desc_size);
            for (size_t i = 0; i < shader_desc_.cb_desc.size(); ++i)
            {
                res.read(reinterpret_cast<char*>(&len), sizeof(len));
                shader_desc_.cb_desc[i].name.resize(len);
                res.read(&shader_desc_.cb_desc[i].name[0], len);

                shader_desc_.cb_desc[i].name_hash = RtHash(shader_desc_.cb_desc[i].name.c_str());

                res.read(reinterpret_cast<char*>(&shader_desc_.cb_desc[i].size), sizeof(shader_desc_.cb_desc[i].size));
                shader_desc_.cb_desc[i].size = LE2Native(shader_desc_.cb_desc[i].size);

                uint16_t var_desc_size;
                res.read(reinterpret_cast<char*>(&var_desc_size), sizeof(var_desc_size));
                var_desc_size = LE2Native(var_desc_size);
                shader_desc_.cb_desc[i].var_desc.resize(var_desc_size);
                for (size_t j = 0; j < shader_desc_.cb_desc[i].var_desc.size(); ++j)
                {
                    res.read(reinterpret_cast<char*>(&len), sizeof(len));
                    shader_desc_.cb_desc[i].var_desc[j].name.resize(len);
                    res.read(&shader_desc_.cb_desc[i].var_desc[j].name[0], len);

                    res.read(reinterpret_cast<char*>(&shader_desc_.cb_desc[i].var_desc[j].start_offset),
                        sizeof(shader_desc_.cb_desc[i].var_desc[j].start_offset));
                    shader_desc_.cb_desc[i].var_desc[j].start_offset = LE2Native(shader_desc_.cb_desc[i].var_desc[j].start_offset);
                    res.read(reinterpret_cast<char*>(&shader_desc_.cb_desc[i].var_desc[j].type),
                        sizeof(shader_desc_.cb_desc[i].var_desc[j].type));
                    res.read(reinterpret_cast<char*>(&shader_desc_.cb_desc[i].var_desc[j].rows),
                        sizeof(shader_desc_.cb_desc[i].var_desc[j].rows));
                    res.read(reinterpret_cast<char*>(&shader_desc_.cb_desc[i].var_desc[j].columns),
                        sizeof(shader_desc_.cb_desc[i].var_desc[j].columns));
                    res.read(reinterpret_cast<char*>(&shader_desc_.cb_desc[i].var_desc[j].elements),
                        sizeof(shader_desc_.cb_desc[i].var_desc[j].elements));
                    shader_desc_.cb_desc[i].var_desc[j].elements = LE2Native(shader_desc_.cb_desc[i].var_desc[j].elements);
                }
            }

            res.read(reinterpret_cast<char*>(&shader_desc_.num_samplers), sizeof(shader_desc_.num_samplers));
            shader_desc_.num_samplers = LE2Native(shader_desc_.num_samplers);
            res.read(reinterpret_cast<char*>(&shader_desc_.num_srvs), sizeof(shader_desc_.num_srvs));
            shader_desc_.num_srvs = LE2Native(shader_desc_.num_srvs);
            res.read(reinterpret_cast<char*>(&shader_desc_.num_uavs), sizeof(shader_desc_.num_uavs));
            shader_desc_.num_uavs = LE2Native(shader_desc_.num_uavs);

            uint16_t res_desc_size;
            res.read(reinterpret_cast<char*>(&res_desc_size), sizeof(res_desc_size));
            res_desc_size = LE2Native(res_desc_size);
            shader_desc_.res_desc.resize(res_desc_size);
            for (size_t i = 0; i < shader_desc_.res_desc.size(); ++i)
            {
                res.read(reinterpret_cast<char*>(&len), sizeof(len));
                shader_desc_.res_desc[i].name.resize(len);
                res.read(&shader_desc_.res_desc[i].name[0], len);

                res.read(reinterpret_cast<char*>(&shader_desc_.res_desc[i].type), sizeof(shader_desc_.res_desc[i].type));

                res.read(reinterpret_cast<char*>(&shader_desc_.res_desc[i].dimension), sizeof(shader_desc_.res_desc[i].dimension));

                res.read(reinterpret_cast<char*>(&shader_desc_.res_desc[i].bind_point), sizeof(shader_desc_.res_desc[i].bind_point));
                shader_desc_.res_desc[i].bind_point = LE2Native(shader_desc_.res_desc[i].bind_point);
            }

            this->FillCBufferIndices(effect);

            this->StageSpecificStreamIn(res);
        }
    }
    else
    {
        std::vector<char> native_shader_block(native_shader_block_len);
        res.read(reinterpret_cast<char*>(native_shader_block.data()), native_shader_block_len);
    }
}

void D3D11ShaderStageObject::StreamOut(std::ostream& os)
{
    std::vector<char> native_shader_block;
    VectorOutputStreamBuf native_shader_buff(native_shader_block);
    std::ostream oss(&native_shader_buff);

    {
        uint8_t len = static_cast<uint8_t>(shader_profile_.size());
        oss.write(reinterpret_cast<char const*>(&len), sizeof(len));
        oss.write(reinterpret_cast<char const*>(shader_profile_.data()), len);
    }

    if (!shader_code_.empty())
    {
        uint8_t len;

        uint32_t blob_size = Native2LE(static_cast<uint32_t>(shader_code_.size()));
        oss.write(reinterpret_cast<char const*>(&blob_size), sizeof(blob_size));
        oss.write(reinterpret_cast<char const*>(shader_code_.data()), shader_code_.size());

        uint16_t cb_desc_size = Native2LE(static_cast<uint16_t>(shader_desc_.cb_desc.size()));
        oss.write(reinterpret_cast<char const*>(&cb_desc_size), sizeof(cb_desc_size));
        for (size_t i = 0; i < shader_desc_.cb_desc.size(); ++i)
        {
            len = static_cast<uint8_t>(shader_desc_.cb_desc[i].name.size());
            oss.write(reinterpret_cast<char const*>(&len), sizeof(len));
            oss.write(reinterpret_cast<char const*>(&shader_desc_.cb_desc[i].name[0]), len);

            uint32_t size = Native2LE(shader_desc_.cb_desc[i].size);
            oss.write(reinterpret_cast<char const*>(&size), sizeof(size));

            uint16_t var_desc_size = Native2LE(static_cast<uint16_t>(shader_desc_.cb_desc[i].var_desc.size()));
            oss.write(reinterpret_cast<char const*>(&var_desc_size), sizeof(var_desc_size));
            for (size_t j = 0; j < shader_desc_.cb_desc[i].var_desc.size(); ++j)
            {
                len = static_cast<uint8_t>(shader_desc_.cb_desc[i].var_desc[j].name.size());
                oss.write(reinterpret_cast<char const*>(&len), sizeof(len));
                oss.write(reinterpret_cast<char const*>(&shader_desc_.cb_desc[i].var_desc[j].name[0]), len);

                uint32_t start_offset = Native2LE(shader_desc_.cb_desc[i].var_desc[j].start_offset);
                oss.write(reinterpret_cast<char const*>(&start_offset), sizeof(start_offset));
                oss.write(reinterpret_cast<char const*>(&shader_desc_.cb_desc[i].var_desc[j].type),
                    sizeof(shader_desc_.cb_desc[i].var_desc[j].type));
                oss.write(reinterpret_cast<char const*>(&shader_desc_.cb_desc[i].var_desc[j].rows),
                    sizeof(shader_desc_.cb_desc[i].var_desc[j].rows));
                oss.write(reinterpret_cast<char const*>(&shader_desc_.cb_desc[i].var_desc[j].columns),
                    sizeof(shader_desc_.cb_desc[i].var_desc[j].columns));
                uint16_t elements = Native2LE(shader_desc_.cb_desc[i].var_desc[j].elements);
                oss.write(reinterpret_cast<char const*>(&elements), sizeof(elements));
            }
        }

        uint16_t num_samplers = Native2LE(shader_desc_.num_samplers);
        oss.write(reinterpret_cast<char const*>(&num_samplers), sizeof(num_samplers));
        uint16_t num_srvs = Native2LE(shader_desc_.num_srvs);
        oss.write(reinterpret_cast<char const*>(&num_srvs), sizeof(num_srvs));
        uint16_t num_uavs = Native2LE(shader_desc_.num_uavs);
        oss.write(reinterpret_cast<char const*>(&num_uavs), sizeof(num_uavs));

        uint16_t res_desc_size = Native2LE(static_cast<uint16_t>(shader_desc_.res_desc.size()));
        oss.write(reinterpret_cast<char const*>(&res_desc_size), sizeof(res_desc_size));
        for (size_t i = 0; i < shader_desc_.res_desc.size(); ++i)
        {
            len = static_cast<uint8_t>(shader_desc_.res_desc[i].name.size());
            oss.write(reinterpret_cast<char const*>(&len), sizeof(len));
            oss.write(reinterpret_cast<char const*>(&shader_desc_.res_desc[i].name[0]), len);

            oss.write(reinterpret_cast<char const*>(&shader_desc_.res_desc[i].type), sizeof(shader_desc_.res_desc[i].type));
            oss.write(reinterpret_cast<char const*>(&shader_desc_.res_desc[i].dimension), sizeof(shader_desc_.res_desc[i].dimension));
            uint16_t bind_point = Native2LE(shader_desc_.res_desc[i].bind_point);
            oss.write(reinterpret_cast<char const*>(&bind_point), sizeof(bind_point));
        }

        this->StageSpecificStreamOut(oss);
    }

    uint32_t len = static_cast<uint32_t>(native_shader_block.size());
    {
        uint32_t tmp = Native2LE(len);
        os.write(reinterpret_cast<char const*>(&tmp), sizeof(tmp));
    }
    if (len > 0)
    {
        os.write(reinterpret_cast<char const*>(&native_shader_block[0]), len * sizeof(native_shader_block[0]));
    }
}

void D3D11ShaderStageObject::CompileShader(const RenderEffect& effect, const RenderTechnique& tech, const RenderPass& pass,
            const std::array<uint32_t, ShaderStageNum>& shader_desc_ids)
{
    shader_code_.clear();

    const uint32_t shader_desc_id = shader_desc_ids[std::to_underlying(stage_)];
    const auto& sd = effect.GetShaderDesc(shader_desc_id);


    shader_profile_ = std::string(GetShaderProfile(effect, shader_desc_id));
    is_validate_ = !shader_profile_.empty();

    if (is_validate_)
    {
        std::vector<std::pair<char const*, char const*>> macros;
        uint32_t flags = D3DCOMPILE_ENABLE_STRICTNESS;
        
    #if !defined(ZENGINE_DEBUG)
        flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
    #else
        // 设置 D3DCOMPILE_DEBUG 标志用于获取着色器调试信息。该标志可以提升调试体验，
        // 但仍然允许着色器进行优化操作
        flags |= D3DCOMPILE_DEBUG;
        // 在Debug环境下禁用优化以避免出现一些不合理的情况
        flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
    #endif
    
        com_ptr<ID3D11ShaderReflection> reflection;
        shader_code_ = ShaderStageObject::CompileToDXBC(
            stage_, effect, tech, pass, macros, sd.func_name.c_str(), shader_profile_.c_str(), flags, reflection.put_void(), true); 
        
        if(!shader_code_.empty())
        {
            if (reflection != nullptr)
            {
                // 着色器本身的信息
                D3D11_SHADER_DESC desc;
                reflection->GetDesc(&desc);

                for (UINT c = 0; c < desc.ConstantBuffers; ++c)
                {
                    ID3D11ShaderReflectionConstantBuffer* reflection_cb = reflection->GetConstantBufferByIndex(c);
                    // 着色器的常量缓冲区
                    D3D11_SHADER_BUFFER_DESC d3d_cb_desc;
                    reflection_cb->GetDesc(&d3d_cb_desc);
                    if ((D3D_CT_CBUFFER == d3d_cb_desc.Type) || (D3D_CT_TBUFFER == d3d_cb_desc.Type))
                    {
                        auto& cb_desc = shader_desc_.cb_desc.emplace_back();
                        cb_desc.name = d3d_cb_desc.Name;
                        cb_desc.name_hash = RtHash(d3d_cb_desc.Name);
                        cb_desc.size = d3d_cb_desc.Size;

                        // 变量的反射
                        for (UINT v = 0; v < d3d_cb_desc.Variables; ++v)
                        {
                            ID3D11ShaderReflectionVariable* reflection_var = reflection_cb->GetVariableByIndex(v);

                            // 着色器的变量
                            D3D11_SHADER_VARIABLE_DESC var_desc;
                            reflection_var->GetDesc(&var_desc);
                            // 描述着色器变量类型
                            D3D11_SHADER_TYPE_DESC type_desc;
                            reflection_var->GetType()->GetDesc(&type_desc);

                            auto& vd = cb_desc.var_desc.emplace_back();
                            vd.name = var_desc.Name;
                            vd.start_offset = var_desc.StartOffset;
                            vd.type = static_cast<uint8_t>(type_desc.Type);
                            vd.rows = static_cast<uint8_t>(type_desc.Rows);
                            vd.columns = static_cast<uint8_t>(type_desc.Columns);
                            vd.elements = static_cast<uint16_t>(type_desc.Elements);
                        }
                    }
                }
                
                FillCBufferIndices(effect);

                int max_sampler_bind_pt = -1;
                int max_srv_bind_pt = -1;
                int max_uav_bind_pt = -1;
                // 描述着色器资源如何绑定到着色器输入
                for (uint32_t i = 0; i < desc.BoundResources; ++i)
                {
                    D3D11_SHADER_INPUT_BIND_DESC si_desc;
                    reflection->GetResourceBindingDesc(i, &si_desc);

                    switch (si_desc.Type)
                    {
                    case D3D_SIT_SAMPLER:
                        max_sampler_bind_pt = std::max(max_sampler_bind_pt, static_cast<int>(si_desc.BindPoint));
                        break;

                    case D3D_SIT_TEXTURE:
                    case D3D_SIT_STRUCTURED:
                    case D3D_SIT_BYTEADDRESS:
                        max_srv_bind_pt = std::max(max_srv_bind_pt, static_cast<int>(si_desc.BindPoint));
                        break;

                    case D3D_SIT_UAV_RWTYPED:
                    case D3D_SIT_UAV_RWSTRUCTURED:
                    case D3D_SIT_UAV_RWBYTEADDRESS:
                    case D3D_SIT_UAV_APPEND_STRUCTURED:
                    case D3D_SIT_UAV_CONSUME_STRUCTURED:
                    case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
                        max_uav_bind_pt = std::max(max_uav_bind_pt, static_cast<int>(si_desc.BindPoint));
                        break;

                    default:
                        break;
                    }
                }
                shader_desc_.num_samplers = static_cast<uint16_t>(max_sampler_bind_pt + 1);
                shader_desc_.num_srvs = static_cast<uint16_t>(max_srv_bind_pt + 1);
                shader_desc_.num_uavs = static_cast<uint16_t>(max_uav_bind_pt + 1);

                for (uint32_t i = 0; i < desc.BoundResources; ++i)
                {
                    D3D11_SHADER_INPUT_BIND_DESC si_desc;
                    reflection->GetResourceBindingDesc(i, &si_desc);

                    switch (si_desc.Type)
                    {
                    case D3D_SIT_TEXTURE:
                    case D3D_SIT_SAMPLER:
                    case D3D_SIT_STRUCTURED:
                    case D3D_SIT_BYTEADDRESS:
                    case D3D_SIT_UAV_RWTYPED:
                    case D3D_SIT_UAV_RWSTRUCTURED:
                    case D3D_SIT_UAV_RWBYTEADDRESS:
                    case D3D_SIT_UAV_APPEND_STRUCTURED:
                    case D3D_SIT_UAV_CONSUME_STRUCTURED:
                    case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
                        if (effect.ParameterByName(si_desc.Name))
                        {
                            auto& brd = shader_desc_.res_desc.emplace_back();
                            brd.name = si_desc.Name;
                            brd.type = static_cast<uint8_t>(si_desc.Type);
                            brd.bind_point = static_cast<uint16_t>(si_desc.BindPoint);
                        }
                        break;

                    default:
                        break;
                    }
                }

                StageSpecificReflection(reflection.get());
            }
                
            
        }
    }

}

void D3D11ShaderStageObject::CreateHwShader(const RenderEffect& effect, const std::array<uint32_t, ShaderStageNum>& shader_desc_ids)
{
    if (!shader_code_.empty())
    {
        //const ShaderDesc& sd = effect.GetShaderDesc(shader_desc_ids[std::to_underlying(stage_)]);
        is_validate_ = true;
        StageSpecificCreateHwShader(effect, shader_desc_ids);
    }
    else
    {
        is_validate_ = false;
        ClearHwShader();
    }

    hw_res_ready_ = true;
}

std::span<uint8_t const> D3D11ShaderStageObject::ShaderCodeBlob() const
{
    return MakeSpan(shader_code_);
}

void D3D11ShaderStageObject::FillCBufferIndices(RenderEffect const& effect)
{
    if (!shader_desc_.cb_desc.empty())
    {
        cbuff_indices_.resize(shader_desc_.cb_desc.size());
    }
    for (size_t c = 0; c < shader_desc_.cb_desc.size(); ++c)
    {
        uint32_t i = 0;
        for (; i < effect.NumCBuffers(); ++i)
        {
            if (effect.CBufferByIndex(i)->NameHash() == shader_desc_.cb_desc[c].name_hash)
            {
                cbuff_indices_[c] = static_cast<uint8_t>(i);
                break;
            }
        }
        COMMON_ASSERT(i < effect.NumCBuffers());
    }
}

// 创建带流输出阶段的几何着色器
ID3D11GeometryShaderPtr D3D11ShaderStageObject::CreateGeometryShaderWithStreamOutput(const RenderEffect& effect,
    std::array<uint32_t, ShaderStageNum> const& shader_desc_ids, 
    std::span<uint8_t const> code_blob,
    const std::vector<ShaderDesc::StreamOutputDecl>& so_decl)
{
    COMMON_ASSERT(!code_blob.empty());

    const auto& d3d11_re = checked_cast<const D3D11RenderEngine&>(
        Context::Instance().RenderFactoryInstance().RenderEngineInstance());
    auto d3d_device = d3d11_re.D3DDevice1();

    // [In]D3D11_SO_DECLARATION_ENTRY的数组
    std::vector<D3D11_SO_DECLARATION_ENTRY> d3d11_decl(so_decl.size());
    for (size_t i = 0; i < so_decl.size(); ++i)
    {
        d3d11_decl[i] = D3D11Mapping::Mapping(so_decl[i]);
    }

    // [In]按索引指定哪个流输出对象用于传递到光栅化阶段
    UINT rasterized_stream = 0;
    if ((effect.GetShaderDesc(shader_desc_ids[std::to_underlying(ShaderStage::Pixel)]).func_name.empty()))
    {
        rasterized_stream = D3D11_SO_NO_RASTERIZED_STREAM;
    }

    ID3D11GeometryShaderPtr gs;
    if (FAILED(d3d_device->CreateGeometryShaderWithStreamOutput(code_blob.data(), code_blob.size(), &d3d11_decl[0],
            static_cast<UINT>(d3d11_decl.size()), nullptr, 0, rasterized_stream, nullptr, gs.put())))
    {
        is_validate_ = false;
    }

    return gs;
}

std::string_view D3D11ShaderStageObject::GetShaderProfile(RenderEffect const& effect, uint32_t shader_desc_id) const 
{
    std::string_view shader_profile = effect.GetShaderDesc(shader_desc_id).profile;
    if (is_available_)
    {
        if (shader_profile == "auto")
        {
            auto& d3d11_re = checked_cast<D3D11RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
            shader_profile = d3d11_re.DefaultShaderProfile(stage_);
        }
    }
    else
    {
        shader_profile = std::string_view();
    }
    return shader_profile;
}

D3D11VertexShaderStageObject::D3D11VertexShaderStageObject()
    : D3D11ShaderStageObject(ShaderStage::Vertex)
{
    is_available_ = true;
}

void D3D11VertexShaderStageObject::ClearHwShader()
{
    vertex_shader_.reset();
    geometry_shader_.reset();
}

void D3D11VertexShaderStageObject::StageSpecificStreamIn(ResIdentifier& res)
{
    res.read(reinterpret_cast<char*>(&vs_signature_), sizeof(vs_signature_));
    vs_signature_ = LE2Native(vs_signature_);
}

void D3D11VertexShaderStageObject::StageSpecificStreamOut(std::ostream& os)
{
    uint32_t const vs_signature = Native2LE(vs_signature_);
    os.write(reinterpret_cast<char const*>(&vs_signature), sizeof(vs_signature));
}

void D3D11VertexShaderStageObject::StageSpecificCreateHwShader(const RenderEffect& effect, const std::array<uint32_t, ShaderStageNum>& shader_desc_ids)
{
    auto& d3d11_re = checked_cast<D3D11RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
    auto d3d_device = d3d11_re.D3DDevice1();

    if (FAILED(d3d_device->CreateVertexShader(shader_code_.data(), shader_code_.size(), nullptr, vertex_shader_.put())))
    {
        is_validate_ = false;
    }
    else
    {
		const ShaderDesc& sd = effect.GetShaderDesc(shader_desc_ids[std::to_underlying(stage_)]);
        const auto& caps = d3d11_re.DeviceCaps();
        if (!sd.so_decl.empty())
        {
            if (caps.gs_support)
            {
                geometry_shader_ = CreateGeometryShaderWithStreamOutput(effect, shader_desc_ids, shader_code_, sd.so_decl);
            }
            else
            {
                is_validate_ = false;
            }
        }
    }
}

#if ZENGINE_IS_DEV_PLATFORM
void D3D11VertexShaderStageObject::StageSpecificReflection(ID3D11ShaderReflection* reflection)
{
    D3D11_SHADER_DESC desc;
    reflection->GetDesc(&desc);

    vs_signature_ = 0;
    for (uint32_t i = 0; i < desc.InputParameters; ++i)
    {
        D3D11_SIGNATURE_PARAMETER_DESC signature;
        reflection->GetInputParameterDesc(i, &signature);

        size_t seed = RtHash(signature.SemanticName);
        HashCombine(seed, signature.SemanticIndex);
        HashCombine(seed, signature.Register);
        HashCombine(seed, static_cast<uint32_t>(signature.SystemValueType));
        HashCombine(seed, static_cast<uint32_t>(signature.ComponentType));
        HashCombine(seed, signature.Mask);
        HashCombine(seed, signature.ReadWriteMask);
        HashCombine(seed, signature.Stream);
        HashCombine(seed, signature.MinPrecision);

        size_t sig = vs_signature_;
        HashCombine(sig, seed);
        vs_signature_ = static_cast<uint32_t>(sig);
    }
}
#endif

D3D11PixelShaderStageObject::D3D11PixelShaderStageObject()
    : D3D11ShaderStageObject(ShaderStage::Pixel)
{
    is_available_ = true;
}

void D3D11PixelShaderStageObject::ClearHwShader()
{
    pixel_shader_.reset();
}

void D3D11PixelShaderStageObject::StageSpecificCreateHwShader(const RenderEffect& effect, const std::array<uint32_t, ShaderStageNum>& shader_desc_ids)
{
    const auto& d3d11_re = checked_cast<const D3D11RenderEngine&>(
        Context::Instance().RenderFactoryInstance().RenderEngineInstance());
    auto d3d_device = d3d11_re.D3DDevice1();
    if (FAILED(d3d_device->CreatePixelShader(shader_code_.data(), shader_code_.size(), nullptr, pixel_shader_.put())))
    {
        is_validate_ = false;
    }
}

D3D11GeometryShaderStageObject::D3D11GeometryShaderStageObject()
    : D3D11ShaderStageObject(ShaderStage::Geometry)
{
    is_available_ = true;
}

void D3D11GeometryShaderStageObject::ClearHwShader()
{
    geometry_shader_.reset();
}

void D3D11GeometryShaderStageObject::StageSpecificCreateHwShader(const RenderEffect& effect, const std::array<uint32_t, ShaderStageNum>& shader_desc_ids)
{
    if (is_available_)
    {
        const ShaderDesc& sd = effect.GetShaderDesc(shader_desc_ids[std::to_underlying(stage_)]);
        if (sd.so_decl.empty())
        {
            const auto& re = checked_cast<const D3D11RenderEngine&>(
                Context::Instance().RenderFactoryInstance().RenderEngineInstance());
            auto d3d_device = re.D3DDevice1();

            if (FAILED(d3d_device->CreateGeometryShader(shader_code_.data(), shader_code_.size(), nullptr, geometry_shader_.put())))
            {
                is_validate_ = false;
            }
        }
        else
        {
            geometry_shader_ = CreateGeometryShaderWithStreamOutput(effect, shader_desc_ids, shader_code_, sd.so_decl);
        }
    }
    else
    {
        is_validate_ = false;
    }

}




D3D11ShaderObject::D3D11ShaderObject()
    : D3D11ShaderObject(MakeSharedPtr<Immutable>(), MakeSharedPtr<D3D11Immutable>())
{
}

D3D11ShaderObject::D3D11ShaderObject(std::shared_ptr<Immutable> immutable, std::shared_ptr<D3D11Immutable> d3d_immutable) noexcept
    :ShaderObject(std::move(immutable)), d3d_immutable_(std::move(d3d_immutable))
{
    
}

ShaderObjectPtr D3D11ShaderObject::Clone(RenderEffect& dst_effect)
{
    auto ret = MakeSharedPtr<D3D11ShaderObject>(immutable_, d3d_immutable_);

    ret->hw_res_ready_ = hw_res_ready_;
    ret->uavsrcs_.resize(uavsrcs_.size(), nullptr);
    ret->uavs_.resize(uavs_.size());
    ret->uav_init_counts_.resize(uav_init_counts_.size());

    for (size_t i = 0; i < ShaderStageNum; ++ i)
    {
        ret->srvsrcs_[i].resize(srvsrcs_[i].size(), std::make_tuple(static_cast<void*>(nullptr), 0, 0));
        ret->srvs_[i].resize(srvs_[i].size());

        ret->param_binds_[i].reserve(param_binds_[i].size());
        for (auto const & pb : param_binds_[i])
        {
            ret->param_binds_[i].push_back(ret->GetBindFunc(static_cast<ShaderStage>(i), pb.offset,
                *dst_effect.ParameterByName(pb.param->Name())));
        }
    }

    return ret;
}

void D3D11ShaderObject::Bind(const RenderEffect& effect)
{
    auto& d3d11_re = checked_cast<D3D11RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

    auto const& vs_stage = Stage(ShaderStage::Vertex);
    d3d11_re.VSSetShader(vs_stage ? checked_cast<D3D11ShaderStageObject&>(*vs_stage).HwVertexShader() : nullptr);

    auto const& ps_stage = Stage(ShaderStage::Pixel);
	d3d11_re.PSSetShader(ps_stage ? checked_cast<D3D11ShaderStageObject&>(*ps_stage).HwPixelShader() : nullptr);

    ShaderStage stream_output_stage = ShaderStage::NumStages;
    if (Stage(ShaderStage::Geometry))
    {
        stream_output_stage = ShaderStage::Geometry;
    }
    else if (Stage(ShaderStage::Vertex))
    {
        stream_output_stage = ShaderStage::Vertex;
    }
    d3d11_re.GSSetShader((stream_output_stage != ShaderStage::NumStages)
        ? checked_cast<D3D11ShaderStageObject&>(*Stage(stream_output_stage)).HwGeometryShader()
        : nullptr);

    for (auto const & pbs : param_binds_)
    {
        for (auto const & pb : pbs)
        {
            pb.update();
        }
    }

    for (size_t stage_index = 0; stage_index < ShaderStageNum; ++stage_index)
    {
        const ShaderStage stage = static_cast<ShaderStage>(stage_index);
        const auto* shader_stage = checked_cast<D3D11ShaderStageObject*>(Stage(static_cast<ShaderStage>(stage)).get());
        if (!shader_stage)
        {
            continue;
        }

        if (!srvs_[stage_index].empty())
        {
            // 绑定着色器资源
            d3d11_re.SetShaderResources(stage, srvsrcs_[stage_index], srvs_[stage_index]);
        }

        if (!d3d_immutable_->samplers_[stage_index].empty())
        {
            // 绑定取样器
            d3d11_re.SetSamplers(stage, d3d_immutable_->samplers_[stage_index]);
        }

        auto const& cbuff_indices = shader_stage->CBufferIndices();
        if(cbuff_indices.empty())
        {
            continue;
        }
        ID3D11Buffer* d3d11_cbuffs[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT];
        for (uint32_t i = 0; i < cbuff_indices.size(); ++i)
        {
            // 刷新常量
            auto* cb = effect.CBufferByIndex(cbuff_indices[i]);
            cb->Update();
            d3d11_cbuffs[i] = checked_cast<D3D11GraphicsBuffer*>(cb->HWBuff().get())->D3DBuffer();
        }

        d3d11_re.SetConstantBuffers(stage, MakeSpan(d3d11_cbuffs, cbuff_indices.size()));
    }
}

void D3D11ShaderObject::Unbind()
{
    
}

std::span<uint8_t const> D3D11ShaderObject::VsCode() const
{
    return checked_cast<D3D11ShaderStageObject&>(*Stage(ShaderStage::Vertex)).ShaderCodeBlob();
}

uint32_t D3D11ShaderObject::VsSignature() const noexcept
{
    return checked_cast<D3D11VertexShaderStageObject&>(*Stage(ShaderStage::Vertex)).VsSignature();
}

void D3D11ShaderObject::DoLinkShaders(RenderEffect& effect)
{
    for (size_t stage = 0; stage < ShaderStageNum; ++stage)
    {
        const auto* shader_stage = checked_cast<D3D11ShaderStageObject*>(Stage(static_cast<ShaderStage>(stage)).get());
        if (nullptr == shader_stage)
        {
            continue;
        }
        if (!shader_stage->ShaderCodeBlob().empty())
        {
            const auto& shader_desc = shader_stage->GetD3D11ShaderDesc();
            d3d_immutable_->samplers_[stage].resize(shader_desc.num_samplers);
            srvsrcs_[stage].resize(shader_desc.num_srvs, std::make_tuple(static_cast<void*>(nullptr), 0, 0));
            srvs_[stage].resize(shader_desc.num_srvs);

            for (size_t i = 0; i < shader_desc.res_desc.size(); ++i)
            {
                RenderEffectParameter* p = effect.ParameterByName(shader_desc.res_desc[i].name);
                COMMON_ASSERT(p);
    
                uint32_t offset = shader_desc.res_desc[i].bind_point;
                if (D3D_SIT_SAMPLER == shader_desc.res_desc[i].type)
                {
                    SamplerStateObjectPtr sampler;
                    p->Value(sampler);
                    if (sampler)
                    {
                        // 获取取样器
                        d3d_immutable_->samplers_[stage][offset] = checked_cast<D3D11SamplerStateObject&>(*sampler).D3DSamplerState();
                    }
                }
                else
                {
                    // 获取着色器资源
                    param_binds_[stage].push_back(GetBindFunc(static_cast<ShaderStage>(stage), offset, *p));
                }
            }
        }

        if (!shader_stage->CBufferIndices().empty())
        {
            auto const& shader_desc = shader_stage->GetD3D11ShaderDesc();
            auto const& cbuff_indices = shader_stage->CBufferIndices();

            for (size_t i = 0; i < cbuff_indices.size(); ++i)
            {
                auto cbuff = effect.CBufferByIndex(cbuff_indices[i]);
                cbuff->Resize(shader_desc.cb_desc[i].size);
                COMMON_ASSERT(cbuff->NumParameters() == shader_desc.cb_desc[i].var_desc.size());
                for (uint32_t j = 0; j < cbuff->NumParameters(); ++j)
                {
                    RenderEffectParameter* param = effect.ParameterByIndex(cbuff->ParameterIndex(j));
                    uint32_t stride;
                    if (param->Type() == REDT_struct)
                    {
                        stride = 1;
                    }
                    else if (shader_desc.cb_desc[i].var_desc[j].elements > 0)
                    {
                        if (param->Type() != REDT_float4x4)
                        {
                            stride = 16;
                        }
                        else
                        {
                            stride = 64;
                        }
                    }
                    else
                    {
                        if (param->Type() != REDT_float4x4)
                        {
                            stride = 4;
                        }
                        else
                        {
                            stride = 16;
                        }
                    }
                    param->BindToCBuffer(effect, cbuff_indices[i], shader_desc.cb_desc[i].var_desc[j].start_offset, stride);
                }
            }
        }
    }
}

D3D11ShaderObject::ParameterBind D3D11ShaderObject::GetBindFunc(ShaderStage stage, uint32_t offset, RenderEffectParameter const& param)
{
    uint32_t const stage_index = std::to_underlying(stage);

    ParameterBind ret;
    ret.param = &param;
    ret.offset = offset;

    switch (param.Type())
    {
    case REDT_bool:
    case REDT_uint:
    case REDT_int:
    case REDT_float:
    case REDT_uint2:
    case REDT_uint3:
    case REDT_uint4:
    case REDT_int2:
    case REDT_int3:
    case REDT_int4:
    case REDT_float2:
    case REDT_float3:
    case REDT_float4:
    case REDT_float4x4:
    case REDT_sampler:
        break;

    case REDT_texture1D:
    case REDT_texture2D:
    case REDT_texture2DMS:
    case REDT_texture3D:
    case REDT_textureCUBE:
    case REDT_texture1DArray:
    case REDT_texture2DArray:
    case REDT_texture2DMSArray:
    case REDT_texture3DArray:
    case REDT_textureCUBEArray:
    case REDT_buffer:
    case REDT_structured_buffer:
    case REDT_consume_structured_buffer:
    case REDT_append_structured_buffer:
    case REDT_byte_address_buffer:
        ret.update = D3D11ShaderParameterSrvUpdater(srvsrcs_[stage_index][offset], srvs_[stage_index][offset], param);
        break;

    case REDT_rw_texture1D:
    case REDT_rw_texture2D:
    case REDT_rw_texture3D:
    case REDT_rw_texture1DArray:
    case REDT_rw_texture2DArray:
    case REDT_rasterizer_ordered_texture1D:
    case REDT_rasterizer_ordered_texture1DArray:
    case REDT_rasterizer_ordered_texture2D:
    case REDT_rasterizer_ordered_texture2DArray:
    case REDT_rasterizer_ordered_texture3D:
    case REDT_rw_buffer:
    case REDT_rw_structured_buffer:
    case REDT_rw_byte_address_buffer:
    case REDT_rasterizer_ordered_buffer:
    case REDT_rasterizer_ordered_structured_buffer:
    case REDT_rasterizer_ordered_byte_address_buffer:
        //ret.update = D3D11ShaderParameterUavUpdater(uavsrcs_[offset], uavs_[offset], uav_init_counts_[offset], param);
        break;

    default:
        ZENGINE_UNREACHABLE("Invalid type");
    }

    return ret;
}

}