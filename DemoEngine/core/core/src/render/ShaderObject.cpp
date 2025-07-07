#include <common/DllLoader.h>
#include <common/com_ptr.h>
#include <common/CustomizedStreamBuf.h>
#include <Base/Context.h>

#include <Render/ShaderObject.h>

#if ZENGINE_IS_DEV_PLATFORM

#ifdef ZENGINE_PLATFORM_WINDOWS
#define CALL_D3DCOMPILER_DIRECTLY
#endif

#ifdef CALL_D3DCOMPILER_DIRECTLY
#include <d3dx11.h>
#include <d3dcompiler.h>
#endif
#endif


namespace
{
using namespace RenderWorker;
class D3DCompilerLoader
{
public:
    ~D3DCompilerLoader()
    {
#ifdef CALL_D3DCOMPILER_DIRECTLY
        mod_d3dcompiler_.Free();
#endif
    }

    static D3DCompilerLoader& Instance()
    {
        static D3DCompilerLoader initer;
        return initer;
    }

    HRESULT D3DCompile(std::string const & src_data,
        D3D_SHADER_MACRO const * defines, char const * entry_point,
        char const * target, uint32_t flags1, uint32_t flags2,
        std::vector<uint8_t>& code, std::string& error_msgs) const
    {
#ifdef CALL_D3DCOMPILER_DIRECTLY
        com_ptr<ID3DBlob> code_blob;
        com_ptr<ID3DBlob> error_msgs_blob;
        HRESULT hr = DynamicD3DCompile_(src_data.c_str(), static_cast<UINT>(src_data.size()),
            nullptr, defines, nullptr, entry_point,
            target, flags1, flags2, code_blob.put(), error_msgs_blob.put());
        if (code_blob)
        {
            uint8_t const * p = static_cast<uint8_t const *>(code_blob->GetBufferPointer());
            code.assign(p, p + code_blob->GetBufferSize());
        }
        else
        {
            code.clear();
        }
        if (error_msgs_blob)
        {
            char const * p = static_cast<char const *>(error_msgs_blob->GetBufferPointer());
            error_msgs.assign(p, p + error_msgs_blob->GetBufferSize());
        }
        else
        {
            error_msgs.clear();
        }
        return hr;
#else
#endif
    }

    HRESULT D3DStripShader(std::vector<uint8_t> const & shader_code, uint32_t strip_flags, std::vector<uint8_t>& stripped_code)
    {
#ifdef CALL_D3DCOMPILER_DIRECTLY
        com_ptr<ID3DBlob> stripped_blob;
        HRESULT hr = DynamicD3DStripShader_(&shader_code[0], static_cast<UINT>(shader_code.size()), strip_flags, stripped_blob.put());

        uint8_t const * p = static_cast<uint8_t const *>(stripped_blob->GetBufferPointer());
        stripped_code.assign(p, p + stripped_blob->GetBufferSize());

        return hr;
#else
        // TODO
        KFL_UNUSED(shader_code);
        KFL_UNUSED(strip_flags);
        KFL_UNUSED(stripped_code);
        return S_OK;
#endif
    }

    HRESULT D3DReflect(std::vector<uint8_t> const & shader_code, void** reflector)
    {
#ifdef CALL_D3DCOMPILER_DIRECTLY
        static GUID const IID_ID3D11ShaderReflection_47
            = { 0x8d536ca1, 0x0cca, 0x4956, { 0xa8, 0x37, 0x78, 0x69, 0x63, 0x75, 0x55, 0x84 } };

        return DynamicD3DReflect_(&shader_code[0],  // [In]编译好的着色器二进制信息
            static_cast<UINT>(shader_code.size()),  // [In]编译好的着色器二进制信息字节数
            IID_ID3D11ShaderReflection_47,          // [In]COM组件的GUID
            reflector);                             // [Out]输出的着色器反射借口
#else
        // TODO
        KFL_UNUSED(shader_code);
        KFL_UNUSED(reflector);
        return S_OK;
#endif
    }

private:
    D3DCompilerLoader()
    {
    #ifdef CALL_D3DCOMPILER_DIRECTLY
        mod_d3dcompiler_.Load("d3dcompiler_47.dll");

        DynamicD3DCompile_ = reinterpret_cast<D3DCompileFunc>(mod_d3dcompiler_.GetProcAddress("D3DCompile"));
        DynamicD3DReflect_ = reinterpret_cast<D3DReflectFunc>(mod_d3dcompiler_.GetProcAddress("D3DReflect"));
        DynamicD3DStripShader_ = reinterpret_cast<D3DStripShaderFunc>(mod_d3dcompiler_.GetProcAddress("D3DStripShader"));
    #endif
}

private:
#ifdef CALL_D3DCOMPILER_DIRECTLY
    typedef HRESULT (WINAPI *D3DCompileFunc)(LPCVOID pSrcData, SIZE_T SrcDataSize, LPCSTR pSourceName,
        D3D_SHADER_MACRO const * pDefines, ID3DInclude* pInclude, LPCSTR pEntrypoint,
        LPCSTR pTarget, UINT Flags1, UINT Flags2, ID3DBlob** ppCode, ID3DBlob** ppErrorMsgs);
    typedef HRESULT (WINAPI *D3DReflectFunc)(LPCVOID pSrcData, SIZE_T SrcDataSize, REFIID pInterface, void** ppReflector);
    typedef HRESULT (WINAPI *D3DStripShaderFunc)(LPCVOID pShaderBytecode, SIZE_T BytecodeLength, UINT uStripFlags,
        ID3DBlob** ppStrippedBlob);

    DllLoader mod_d3dcompiler_;
    D3DCompileFunc DynamicD3DCompile_;
    D3DReflectFunc DynamicD3DReflect_;
    D3DStripShaderFunc DynamicD3DStripShader_;
#endif
};
}

namespace RenderWorker
{

bool operator!=(const ShaderDesc::StreamOutputDecl& lhs, const ShaderDesc::StreamOutputDecl& rhs) noexcept
{
    return !(lhs == rhs);
}

bool operator==(const ShaderDesc::StreamOutputDecl& lhs, const ShaderDesc::StreamOutputDecl& rhs) noexcept
{
    return (lhs.usage == rhs.usage) && (lhs.usage_index == rhs.usage_index)
        && (lhs.start_component == rhs.start_component) && (lhs.component_count == rhs.component_count)
        && (lhs.slot == rhs.slot);
}

bool operator!=(const ShaderDesc& lhs, const ShaderDesc& rhs) noexcept
{
    return !(lhs == rhs);
}

bool operator==(const ShaderDesc& lhs, const ShaderDesc& rhs) noexcept
{
    return (lhs.profile == rhs.profile) && (lhs.func_name == rhs.func_name)
        && (lhs.macros_hash == rhs.macros_hash) && (lhs.so_decl == rhs.so_decl);
    
}

ShaderStageObject::ShaderStageObject(ShaderStage stage) noexcept 
    :stage_(stage)
{
}

ShaderStageObject::~ShaderStageObject() noexcept = default;




ShaderObject::ShaderObject()
    : ShaderObject(MakeSharedPtr<ShaderObject::Immutable>())
{
    
}

ShaderObject::ShaderObject(std::shared_ptr<Immutable> immutable) noexcept
    : immutable_(std::move(immutable))
{
}

ShaderObject::~ShaderObject() noexcept = default;

void ShaderObject::AttachStage(ShaderStage stage, const ShaderStageObjectPtr& shader_stage)
{
    auto& curr_shader_stage = immutable_->shader_stages_[std::to_underlying(stage)];
    if (curr_shader_stage != shader_stage)
    {
        curr_shader_stage = shader_stage;
        shader_stages_dirty_ = true;
        hw_res_ready_ = false;
    }
}

const ShaderStageObjectPtr& ShaderObject::Stage(ShaderStage stage) const noexcept
{
    return immutable_->shader_stages_[std::to_underlying(stage)];
}

void ShaderObject::LinkShaders(RenderEffect& effect)
{
    if (shader_stages_dirty_)
    {
        immutable_->is_validate_ = true;
        for (uint32_t stage_index = 0; stage_index < ShaderStageNum; ++stage_index)
        {
            ShaderStage const stage = static_cast<ShaderStage>(stage_index);
            const auto& shader_stage = this->Stage(stage);
            if (shader_stage)
            {
                immutable_->is_validate_ &= shader_stage->Validate();
            }
        }

        DoLinkShaders(effect);

        shader_stages_dirty_ = false;
        hw_res_ready_ = true;
    }
}

#if ZENGINE_IS_DEV_PLATFORM
	std::vector<uint8_t> ShaderStageObject::CompileToDXBC(ShaderStage stage, RenderEffect const& effect, RenderTechnique const& tech,
		RenderPass const& pass, std::vector<std::pair<char const*, char const*>> const& api_special_macros, char const* func_name,
		char const* shader_profile, uint32_t flags, void** reflector, bool strip)
	{
        //auto& re = Context::Instance().RenderEngineInstance();
        const std::string& hlsl_shader_text = effect.HLSLShaderText();
        std::vector<uint8_t> code;
        
        std::string err_msg;
		std::vector<D3D_SHADER_MACRO> macros;

        {
			char const* type_name;
			switch (stage)
			{
			case ShaderStage::Vertex:
				type_name = "KLAYGE_VERTEX_SHADER";
				break;

			case ShaderStage::Pixel:
				type_name = "KLAYGE_PIXEL_SHADER";
				break;

			case ShaderStage::Geometry:
				type_name = "KLAYGE_GEOMETRY_SHADER";
				break;

			case ShaderStage::Compute:
				type_name = "KLAYGE_COMPUTE_SHADER";
				break;

			case ShaderStage::Hull:
				type_name = "KLAYGE_HULL_SHADER";
				break;

			case ShaderStage::Domain:
				type_name = "KLAYGE_DOMAIN_SHADER";
				break;

			default:
				KFL_UNREACHABLE("Invalid shader stage");
			}
			macros.emplace_back(D3D_SHADER_MACRO{type_name, "1"});
		}
        for (uint32_t i = 0; i < api_special_macros.size(); ++i)
		{
			macros.emplace_back(D3D_SHADER_MACRO{api_special_macros[i].first, api_special_macros[i].second});
		}

        for (uint32_t i = 0; i < tech.NumMacros(); ++i)
		{
			std::pair<std::string, std::string> const & name_value = tech.MacroByIndex(i);
			macros.emplace_back(D3D_SHADER_MACRO{name_value.first.c_str(), name_value.second.c_str()});
		}

        for (uint32_t i = 0; i < pass.NumMacros(); ++i)
		{
			std::pair<std::string, std::string> const & name_value = pass.MacroByIndex(i);
			macros.emplace_back(D3D_SHADER_MACRO{name_value.first.c_str(), name_value.second.c_str()});
		}
        macros.emplace_back(D3D_SHADER_MACRO{nullptr, nullptr});

        D3DCompilerLoader::Instance().D3DCompile(hlsl_shader_text, &macros[0],
			func_name, shader_profile,
			flags, 0, code, err_msg);
        if (!err_msg.empty())
        {

            LOGER_WARN() << "Error when compiling " << func_name << ":" << std::endl;

			std::map<int, std::vector<std::string>> err_lines;
			{
				MemInputStreamBuf err_msg_buff(err_msg.data(), err_msg.size());
				std::istream err_iss(&err_msg_buff);

				std::string err_str;
				while (err_iss)
				{
					std::getline(err_iss, err_str);

					int err_line = -1;
					std::string::size_type pos = err_str.find("): error X");
					if (pos == std::string::npos)
					{
						pos = err_str.find("): warning X");
					}
					if (pos != std::string::npos)
					{
						std::string part_err_str = err_str.substr(0, pos);
						pos = part_err_str.rfind("(");
						part_err_str = part_err_str.substr(pos + 1);
						MemInputStreamBuf stream_buff(part_err_str.data(), part_err_str.size());
						std::istream(&stream_buff) >> err_line;
					}

					std::vector<std::string>& msgs = err_lines[err_line];
					bool found = false;
					for (auto const & msg : msgs)
					{
						if (msg == err_str)
						{
							found = true;
							break;
						}
					}

					if (!found)
					{
						// To make the error message unrecognized by Visual Studio
						if ((0 == err_str.find("error X")) || (0 == err_str.find("warning X")))
						{
							err_str = "(0): " + err_str;
						}

						msgs.emplace_back(std::move(err_str));
					}
				}
			}

            for (auto iter = err_lines.begin(); iter != err_lines.end(); ++iter)
			{
				if (iter->first >= 0)
				{
					MemInputStreamBuf hlsl_buff(hlsl_shader_text.data(), hlsl_shader_text.size());
					std::istream iss(&hlsl_buff);

					std::string s;
					int line = 1;

					LOGER_INFO() << "..." << std::endl;
					while (iss && ((iter->first - line) >= 3))
					{
						std::getline(iss, s);
						++line;
					}
					while (iss && (abs(line - iter->first) < 3))
					{
						std::getline(iss, s);

						while (!s.empty() && (('\r' == s[s.size() - 1]) || ('\n' == s[s.size() - 1])))
						{
							s.resize(s.size() - 1);
						}

						LOGER_INFO() << line << ' ' << s << std::endl;

						++line;
					}
					LOGER_INFO()  << "..." << std::endl;
				}

				for (auto const & msg : iter->second)
				{
					LOGER_INFO() << msg << std::endl;
				}
			}
        }

        if (reflector != nullptr)
		{
			D3DCompilerLoader::Instance().D3DReflect(code, reflector);
		}

        if (strip)
		{
#ifndef ZENGINE_PLATFORM_WINDOWS
			enum D3DCOMPILER_STRIP_FLAGS
			{
				D3DCOMPILER_STRIP_REFLECTION_DATA = 1,
				D3DCOMPILER_STRIP_DEBUG_INFO = 2,
				D3DCOMPILER_STRIP_TEST_BLOBS = 4,
				D3DCOMPILER_STRIP_PRIVATE_DATA = 8,
				D3DCOMPILER_STRIP_ROOT_SIGNATURE = 16,
			};
#endif

#if !defined(_DEBUG)
			const uint32_t strip_flags = D3DCOMPILER_STRIP_REFLECTION_DATA | D3DCOMPILER_STRIP_DEBUG_INFO | D3DCOMPILER_STRIP_TEST_BLOBS |
										 D3DCOMPILER_STRIP_PRIVATE_DATA | D3DCOMPILER_STRIP_ROOT_SIGNATURE;
#else
            const uint32_t strip_flags = D3DCOMPILER_STRIP_REFLECTION_DATA | D3DCOMPILER_STRIP_PRIVATE_DATA | D3DCOMPILER_STRIP_ROOT_SIGNATURE;
#endif
			D3DCompilerLoader::Instance().D3DStripShader(code, strip_flags, code);

		}

		return code;
    }
#endif
}