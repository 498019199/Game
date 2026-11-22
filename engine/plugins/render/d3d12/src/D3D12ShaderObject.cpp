#include "D3D12ShaderObject.h"
#include "D3D12RenderEngine.h"
#include <base/ZEngine.h>
#include <render/RenderEffect.h>
#include <render/RenderFactory.h>
#include <common/Util.h>
#include <common/Hash.h>
#include <common/com_ptr.h>
#include <common/Log.h>
#include <cstring>
#include <cctype>
#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <wrl/client.h>
#include <dxcapi.h>
#include <d3d11shader.h>
#include <d3dcompiler.h>

namespace RenderWorker
{

namespace
{

char const* StageProfileDxil(ShaderStage stage)
{
	switch (stage)
	{
	case ShaderStage::Vertex:
		return "vs_6_0";
	case ShaderStage::Pixel:
		return "ps_6_0";
	default:
		return "";
	}
}

char const* StageProfileDxbc(ShaderStage stage)
{
	switch (stage)
	{
	case ShaderStage::Vertex:
		return "vs_5_0";
	case ShaderStage::Pixel:
		return "ps_5_0";
	default:
		return "";
	}
}

d3d12::ShaderStage ToGpuStage(ShaderStage stage)
{
	return (stage == ShaderStage::Pixel) ? d3d12::SHADERSTAGE_FRAGMENT : d3d12::SHADERSTAGE_VERTEX;
}

using Microsoft::WRL::ComPtr;

class DxcLoader
{
public:
	static DxcLoader& Instance()
	{
		static DxcLoader loader;
		return loader;
	}

	bool Available() const noexcept
	{
		return create_ != nullptr;
	}

	std::vector<uint8_t> Compile(std::string const& source, char const* entry, char const* profile,
		std::vector<std::wstring> const& defines, std::string& err_out)
	{
		err_out.clear();
		std::vector<uint8_t> result;
		if (!Available())
		{
			err_out = "dxcompiler.dll not loaded";
			return result;
		}

		ComPtr<IDxcUtils> utils;
		ComPtr<IDxcCompiler3> compiler;
		if (FAILED(create_(CLSID_DxcUtils, IID_PPV_ARGS(&utils))) ||
			FAILED(create_(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler))))
		{
			err_out = "DxcCreateInstance failed";
			return result;
		}

		ComPtr<IDxcBlobEncoding> source_blob;
		if (FAILED(utils->CreateBlob(source.data(), static_cast<UINT32>(source.size()), DXC_CP_UTF8, &source_blob)))
		{
			err_out = "IDxcUtils::CreateBlob failed";
			return result;
		}

		std::wstring entry_w(entry, entry + std::strlen(entry));
		std::wstring profile_w(profile, profile + std::strlen(profile));

		std::vector<std::wstring> storage;
		storage.reserve(defines.size());
		std::vector<LPCWSTR> args = {L"-E", entry_w.c_str(), L"-T", profile_w.c_str(), L"-Qstrip_reflect",
			L"-Qstrip_debug"};
		for (auto const& d : defines)
		{
			storage.push_back(L"-D" + d);
			args.push_back(storage.back().c_str());
		}

		DxcBuffer buffer{};
		buffer.Ptr = source_blob->GetBufferPointer();
		buffer.Size = source_blob->GetBufferSize();
		buffer.Encoding = DXC_CP_UTF8;

		ComPtr<IDxcResult> dxc_result;
		HRESULT const hr =
			compiler->Compile(&buffer, args.data(), static_cast<UINT32>(args.size()), nullptr, IID_PPV_ARGS(&dxc_result));
		if (FAILED(hr) || !dxc_result)
		{
			err_out = "IDxcCompiler3::Compile failed";
			return result;
		}

		HRESULT status = S_OK;
		dxc_result->GetStatus(&status);

		ComPtr<IDxcBlobUtf8> errors;
		dxc_result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
		if (errors && errors->GetStringLength() > 0)
		{
			err_out.assign(errors->GetStringPointer(), errors->GetStringLength());
		}
		if (FAILED(status))
		{
			return result;
		}

		ComPtr<IDxcBlob> dxil;
		dxc_result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&dxil), nullptr);
		if (!dxil || dxil->GetBufferSize() == 0)
		{
			err_out = "empty DXIL blob";
			return result;
		}

		auto const* bytes = static_cast<uint8_t const*>(dxil->GetBufferPointer());
		result.assign(bytes, bytes + dxil->GetBufferSize());
		return result;
	}

private:
	DxcLoader()
	{
		auto try_load_w = [this](wchar_t const* path) -> bool {
			HMODULE h = ::LoadLibraryW(path);
			if (!h)
			{
				return false;
			}
			auto* proc = reinterpret_cast<DxcCreateInstanceProc>(::GetProcAddress(h, "DxcCreateInstance"));
			if (!proc)
			{
				::FreeLibrary(h);
				return false;
			}
			module_ = h;
			create_ = proc;
			return true;
		};

		wchar_t module_path[MAX_PATH]{};
		HMODULE self = nullptr;
		if (::GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
				reinterpret_cast<LPCWSTR>(&DxcLoader::Instance), &self) &&
			::GetModuleFileNameW(self, module_path, MAX_PATH) > 0)
		{
			std::wstring dir(module_path);
			auto const slash = dir.find_last_of(L"\\/");
			if (slash != std::wstring::npos)
			{
				dir.resize(slash + 1);
			}
			if (try_load_w((dir + L"dxcompiler.dll").c_str()))
			{
				return;
			}
		}

		if (try_load_w(L"dxcompiler.dll"))
		{
			return;
		}

		wchar_t const* redist_paths[] = {
			L"C:\\Program Files (x86)\\Windows Kits\\10\\Redist\\D3D\\x64\\dxcompiler.dll",
			L"C:\\Program Files (x86)\\Windows Kits\\10\\bin\\10.0.26100.0\\x64\\dxcompiler.dll",
			nullptr,
		};
		for (int i = 0; redist_paths[i]; ++i)
		{
			if (try_load_w(redist_paths[i]))
			{
				return;
			}
		}
	}

	~DxcLoader()
	{
		if (module_)
		{
			::FreeLibrary(module_);
			module_ = nullptr;
		}
	}

	HMODULE module_{nullptr};
	DxcCreateInstanceProc create_{nullptr};
};

std::string ApplyRegisterSpaces(std::string hlsl, D3D12ShaderDesc const& desc)
{
	auto inject_cbuffer = [&hlsl](std::string const& name, uint32_t bind) {
		std::string const needle = "cbuffer " + name;
		size_t pos = hlsl.find(needle);
		if (pos == std::string::npos)
		{
			return;
		}
		size_t insert_at = pos + needle.size();
		if (hlsl.compare(insert_at, 11, " : register") == 0)
		{
			return;
		}
		hlsl.insert(insert_at, " : register(b" + std::to_string(bind) + ", KLAYGE_NATIVE_CBV_SPACE)");
	};

	auto inject_resource = [&hlsl](std::string const& name, char letter, uint32_t bind) {
		std::string const needle = " " + name + ";";
		size_t pos = hlsl.find(needle);
		if (pos == std::string::npos)
		{
			return;
		}
		size_t const semi = pos + needle.size() - 1;
		hlsl.insert(semi, " : register(" + std::string(1, letter) + std::to_string(bind) + ", KLAYGE_NATIVE_RES_SPACE)");
	};

	for (auto const& cb : desc.cb_desc)
	{
		inject_cbuffer(cb.name, cb.bind_point);
	}
	for (auto const& res : desc.res_desc)
	{
		if (res.type == static_cast<uint8_t>(D3D_SIT_SAMPLER))
		{
			inject_resource(res.name, 's', res.bind_point);
		}
		else if (res.type == static_cast<uint8_t>(D3D_SIT_TEXTURE) ||
			res.type == static_cast<uint8_t>(D3D_SIT_STRUCTURED) ||
			res.type == static_cast<uint8_t>(D3D_SIT_BYTEADDRESS))
		{
			inject_resource(res.name, 't', res.bind_point);
		}
	}
	return hlsl;
}

// d3d12:: D3D12 input layout always uses TEXCOORD{location}. Remap classic KlayGE
// VS *input* semantics to that scheme. Must match LocationFromUsage() in D3D12RenderLayout.cpp.
// Only the entry-point parameter list before the first `out`/`inout` is rewritten so VS
// outputs (TEXCOORD0..) and the matching PS inputs stay unchanged.
std::string RemapVertexInputSemantics(std::string hlsl, std::string const& entry)
{
	if (entry.empty())
	{
		return hlsl;
	}

	std::string const patterns[] = {"void " + entry + "(", entry + "("};
	size_t paren = std::string::npos;
	for (auto const& p : patterns)
	{
		size_t const at = hlsl.find(p);
		if (at != std::string::npos)
		{
			paren = at + p.size() - 1;
			break;
		}
	}
	if (paren == std::string::npos || hlsl[paren] != '(')
	{
		return hlsl;
	}

	size_t const params_begin = paren + 1;
	size_t params_end = params_begin;
	size_t depth = 1;
	for (size_t i = params_begin; i < hlsl.size() && depth > 0; ++i)
	{
		char const c = hlsl[i];
		if (c == '(')
		{
			++depth;
			continue;
		}
		if (c == ')')
		{
			--depth;
			if (depth == 0)
			{
				params_end = i;
				break;
			}
			continue;
		}
		if (depth != 1)
		{
			continue;
		}

		// End inputs at the first output parameter: ", out" / " out" / ", inout" / " inout".
		size_t j = i;
		if (c == ',')
		{
			++j;
		}
		else if (c != ' ' && c != '\t' && c != '\n' && c != '\r')
		{
			continue;
		}
		while (j < hlsl.size() && (hlsl[j] == ' ' || hlsl[j] == '\t' || hlsl[j] == '\n' || hlsl[j] == '\r'))
		{
			++j;
		}
		auto is_kw = [&](char const* kw, size_t n) {
			return hlsl.compare(j, n, kw) == 0 && (j + n >= hlsl.size() || !(std::isalnum(static_cast<unsigned char>(hlsl[j + n])) || hlsl[j + n] == '_'));
		};
		if (is_kw("out", 3) || is_kw("inout", 5))
		{
			params_end = (c == ',') ? i : j;
			break;
		}
	}

	if (params_end <= params_begin)
	{
		return hlsl;
	}

	std::string inputs = hlsl.substr(params_begin, params_end - params_begin);
	auto replace_all = [](std::string& s, std::string const& from, std::string const& to) {
		size_t at = 0;
		while ((at = s.find(from, at)) != std::string::npos)
		{
			s.replace(at, from.size(), to);
			at += to.size();
		}
	};

	// Remap existing TEXCOORDn first (high→low), before inventing new TEXCOORD0
	// from POSITION — otherwise POSITION→TEXCOORD0 would be shifted again.
	for (int idx = 7; idx >= 0; --idx)
	{
		replace_all(inputs, ": TEXCOORD" + std::to_string(idx), ": TEXCOORD" + std::to_string(6 + idx));
	}
	{
		size_t at = 0;
		std::string const from = ": TEXCOORD";
		while ((at = inputs.find(from, at)) != std::string::npos)
		{
			size_t const after = at + from.size();
			if (after < inputs.size() && std::isdigit(static_cast<unsigned char>(inputs[after])))
			{
				at = after;
				continue;
			}
			inputs.replace(at, from.size(), ": TEXCOORD6");
			at += std::strlen(": TEXCOORD6");
		}
	}

	replace_all(inputs, ": BLENDINDICES", ": TEXCOORD5");
	replace_all(inputs, ": BLENDWEIGHT", ": TEXCOORD4");
	replace_all(inputs, ": BINORMAL", ": TEXCOORD15");
	replace_all(inputs, ": TANGENT", ": TEXCOORD14");
	replace_all(inputs, ": POSITION", ": TEXCOORD0");
	replace_all(inputs, ": NORMAL", ": TEXCOORD1");
	replace_all(inputs, ": COLOR1", ": TEXCOORD3");
	replace_all(inputs, ": COLOR0", ": TEXCOORD2");
	replace_all(inputs, ": COLOR", ": TEXCOORD2");

	hlsl.replace(params_begin, params_end - params_begin, inputs);
	return hlsl;
}

} // namespace

D3D12ShaderStageObject::D3D12ShaderStageObject(ShaderStage stage) : ShaderStageObject(stage)
{
}

D3D12ShaderStageObject::~D3D12ShaderStageObject()
{
	ReleaseGpuShader();
}

void D3D12ShaderStageObject::ReleaseGpuShader()
{
	if (gpu_shader_)
	{
		if (device_lifetime_ && (device_lifetime_->device == gpu_device_))
		{
			d3d12::ReleaseShader(gpu_device_, gpu_shader_);
		}
		gpu_shader_ = nullptr;
	}
	device_lifetime_.reset();
	gpu_device_ = nullptr;
}

void D3D12ShaderStageObject::FillShaderDescFromReflection(void* d3d11_reflection)
{
	shader_desc_ = {};
	cbuff_indices_.clear();
	if (!d3d11_reflection)
	{
		return;
	}

	auto* reflection = static_cast<ID3D11ShaderReflection*>(d3d11_reflection);
	D3D11_SHADER_DESC desc{};
	if (FAILED(reflection->GetDesc(&desc)))
	{
		return;
	}

	for (UINT c = 0; c < desc.ConstantBuffers; ++c)
	{
		ID3D11ShaderReflectionConstantBuffer* reflection_cb = reflection->GetConstantBufferByIndex(c);
		D3D11_SHADER_BUFFER_DESC d3d_cb_desc{};
		reflection_cb->GetDesc(&d3d_cb_desc);
		if ((D3D_CT_CBUFFER != d3d_cb_desc.Type) && (D3D_CT_TBUFFER != d3d_cb_desc.Type))
		{
			continue;
		}

		auto& cb_desc = shader_desc_.cb_desc.emplace_back();
		cb_desc.name = d3d_cb_desc.Name;
		cb_desc.name_hash = RtHash(d3d_cb_desc.Name);
		cb_desc.size = d3d_cb_desc.Size;
		cb_desc.bind_point = static_cast<uint16_t>(c); // overwritten from BoundResources below

		for (UINT v = 0; v < d3d_cb_desc.Variables; ++v)
		{
			ID3D11ShaderReflectionVariable* reflection_var = reflection_cb->GetVariableByIndex(v);
			D3D11_SHADER_VARIABLE_DESC var_desc{};
			reflection_var->GetDesc(&var_desc);
			D3D11_SHADER_TYPE_DESC type_desc{};
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

	int max_sampler_bind_pt = -1;
	int max_srv_bind_pt = -1;
	int max_cbv_bind_pt = -1;
	for (uint32_t i = 0; i < desc.BoundResources; ++i)
	{
		D3D11_SHADER_INPUT_BIND_DESC si_desc{};
		if (FAILED(reflection->GetResourceBindingDesc(i, &si_desc)))
		{
			continue;
		}
		switch (si_desc.Type)
		{
		case D3D_SIT_CBUFFER:
		case D3D_SIT_TBUFFER:
			max_cbv_bind_pt = std::max(max_cbv_bind_pt, static_cast<int>(si_desc.BindPoint));
			for (auto& cb : shader_desc_.cb_desc)
			{
				if (cb.name == si_desc.Name)
				{
					cb.bind_point = static_cast<uint16_t>(si_desc.BindPoint);
					break;
				}
			}
			break;
		case D3D_SIT_SAMPLER:
			max_sampler_bind_pt = std::max(max_sampler_bind_pt, static_cast<int>(si_desc.BindPoint));
			{
				auto& br = shader_desc_.res_desc.emplace_back();
				br.name = si_desc.Name;
				br.type = static_cast<uint8_t>(si_desc.Type);
				br.bind_point = static_cast<uint16_t>(si_desc.BindPoint);
			}
			break;
		case D3D_SIT_TEXTURE:
		case D3D_SIT_STRUCTURED:
		case D3D_SIT_BYTEADDRESS:
			max_srv_bind_pt = std::max(max_srv_bind_pt, static_cast<int>(si_desc.BindPoint));
			{
				auto& br = shader_desc_.res_desc.emplace_back();
				br.name = si_desc.Name;
				br.type = static_cast<uint8_t>(si_desc.Type);
				br.bind_point = static_cast<uint16_t>(si_desc.BindPoint);
			}
			break;
		default:
			break;
		}
	}
	shader_desc_.num_uniform_buffers = static_cast<uint16_t>(max_cbv_bind_pt + 1);
	// The renderer pairs sampler+texture slots; use the larger of sampler/SRV counts.
	shader_desc_.num_samplers = static_cast<uint16_t>(
		std::max(max_sampler_bind_pt, max_srv_bind_pt) + 1);
	shader_desc_.num_srvs = static_cast<uint16_t>(max_srv_bind_pt + 1);
}

void D3D12ShaderStageObject::FillCBufferIndices(RenderEffect const& effect)
{
	cbuff_indices_.clear();
	if (shader_desc_.cb_desc.empty())
	{
		return;
	}

	cbuff_indices_.resize(shader_desc_.cb_desc.size());
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
		if (i >= effect.NumCBuffers())
		{
			LogError() << "[D3D12] CBuffer not found in effect: " << shader_desc_.cb_desc[c].name << std::endl;
			cbuff_indices_[c] = 0xFF;
		}
	}
}

void D3D12ShaderStageObject::StreamIn(const RenderEffect& effect,
	const std::array<uint32_t, ShaderStageNum>& shader_desc_ids, ResIdentifier& res)
{
	ReleaseGpuShader();
	is_validate_ = false;
	hw_res_ready_ = false;
	shader_code_.clear();
	shader_desc_ = {};
	cbuff_indices_.clear();

	// Consume the entire stage block before interpreting it. A rejected block
	// must not leave the next shader stage reading from the middle of this one.
	constexpr uint32_t max_block_size = 64 * 1024 * 1024;
	uint32_t block_size = 0;
	res.read(&block_size, sizeof(block_size));
	if (!res) return;
	block_size = LE2Native(block_size);
	if (block_size > max_block_size)
	{
		res.input_stream().ignore(static_cast<std::streamsize>(block_size));
		return;
	}
	std::string block(block_size, '\0');
	res.read(block.data(), block.size());
	if (!res || block.empty()) return;

	size_t cursor = 0;
	auto take = [&](size_t count) -> std::string_view {
		if (count > block.size() - cursor) throw std::runtime_error("truncated stage block");
		auto const bytes = std::string_view(block).substr(cursor, count);
		cursor += count;
		return bytes;
	};
	auto read_uint = [&]() {
		uint32_t value;
		auto const bytes = take(sizeof(value));
		std::memcpy(&value, bytes.data(), sizeof(value));
		return LE2Native(value);
	};
	auto read_bounded = [&](uint32_t limit) {
		auto const value = read_uint();
		if (value > limit) throw std::runtime_error("stage metadata value " + std::to_string(value) + " exceeds " + std::to_string(limit));
		return value;
	};
	auto read_text = [&]() { return std::string(take(read_bounded(65535))); };

	try
	{
		if (read_uint() != 1 || read_uint() != std::to_underlying(stage_)) return;
		if (stage_ != ShaderStage::Vertex && stage_ != ShaderStage::Pixel) return;
		auto const format = read_uint();
		if (format != d3d12::SHADERFORMAT_DXIL && format != d3d12::SHADERFORMAT_DXBC) return;
		auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		auto const supported = re.Device() ? d3d12::GetShaderFormats(re.Device()) : d3d12::SHADERFORMAT_DXIL;
		if (!(format & supported)) return;
		auto const entry = read_text();
		auto const profile = read_text();
		auto const shader_id = shader_desc_ids[std::to_underlying(stage_)];
		if (entry != effect.GetShaderDesc(shader_id).func_name || profile != GetShaderProfile(effect, shader_id)) return;

		D3D12ShaderDesc description;
		description.num_samplers = static_cast<uint16_t>(read_bounded(65535));
		description.num_srvs = static_cast<uint16_t>(read_bounded(65535));
		description.num_uavs = static_cast<uint16_t>(read_bounded(65535));
		description.num_uniform_buffers = static_cast<uint16_t>(read_bounded(65535));
		description.cb_desc.resize(read_bounded(255));
		for (auto& buffer : description.cb_desc)
		{
			buffer.name = read_text();
			buffer.name_hash = HashValue(buffer.name);
			buffer.size = read_bounded(65536);
			buffer.bind_point = static_cast<uint16_t>(read_bounded(65535));
			buffer.var_desc.resize(read_bounded(4096));
			for (auto& variable : buffer.var_desc)
			{
				variable.name = read_text();
				variable.start_offset = read_uint();
				variable.type = static_cast<uint8_t>(read_bounded(255));
				// Struct reflection may encode aggregate columns (e.g. CameraInfo has 73),
				// so preserve the full stored byte rather than applying vector dimensions.
				variable.rows = static_cast<uint8_t>(read_bounded(255));
				variable.columns = static_cast<uint8_t>(read_bounded(255));
				variable.elements = static_cast<uint16_t>(read_bounded(65535));
				if (variable.start_offset >= buffer.size) return;
			}
		}
		description.res_desc.resize(read_bounded(4096));
		for (auto& resource : description.res_desc)
		{
			resource.name = read_text();
			resource.type = static_cast<uint8_t>(read_bounded(255));
			resource.bind_point = static_cast<uint16_t>(read_bounded(65535));
		}
		auto const code = take(read_bounded(max_block_size));
		if (cursor != block.size() || code.size() < 4 || code.substr(0, 4) != "DXBC") return;

		shader_code_.assign(code.begin(), code.end());
		shader_format_ = format;
		entry_point_ = entry;
		shader_profile_ = profile;
		shader_desc_ = std::move(description);
		FillCBufferIndices(effect);
		if (std::find(cbuff_indices_.begin(), cbuff_indices_.end(), uint8_t(0xFF)) != cbuff_indices_.end())
		{
			shader_code_.clear();
			shader_desc_ = {};
			cbuff_indices_.clear();
			return;
		}
		is_validate_ = true;
	}
	catch (std::runtime_error const& error)
	{
		LogWarn() << "[D3D12] Rejected shader cache: " << error.what() << std::endl;
	}
}

void D3D12ShaderStageObject::StreamOut(std::ostream& os)
{
	std::string block;
	auto write_uint = [&](uint32_t value) {
		value = Native2LE(value);
		block.append(reinterpret_cast<char const*>(&value), sizeof(value));
	};
	auto write_text = [&](std::string const& value) {
		write_uint(static_cast<uint32_t>(value.size()));
		block.append(value);
	};
	write_uint(1); // Stage block format, independent of the containing KFX format.
	write_uint(std::to_underlying(stage_));
	write_uint(shader_format_);
	write_text(entry_point_);
	write_text(shader_profile_);
	write_uint(shader_desc_.num_samplers);
	write_uint(shader_desc_.num_srvs);
	write_uint(shader_desc_.num_uavs);
	write_uint(shader_desc_.num_uniform_buffers);
	write_uint(static_cast<uint32_t>(shader_desc_.cb_desc.size()));
	for (auto const& buffer : shader_desc_.cb_desc)
	{
		write_text(buffer.name);
		write_uint(buffer.size);
		write_uint(buffer.bind_point);
		write_uint(static_cast<uint32_t>(buffer.var_desc.size()));
		for (auto const& variable : buffer.var_desc)
		{
			write_text(variable.name);
			write_uint(variable.start_offset);
			write_uint(variable.type);
			write_uint(variable.rows);
			write_uint(variable.columns);
			write_uint(variable.elements);
		}
	}
	write_uint(static_cast<uint32_t>(shader_desc_.res_desc.size()));
	for (auto const& resource : shader_desc_.res_desc)
	{
		write_text(resource.name);
		write_uint(resource.type);
		write_uint(resource.bind_point);
	}
	write_uint(static_cast<uint32_t>(shader_code_.size()));
	if (!shader_code_.empty()) block.append(reinterpret_cast<char const*>(shader_code_.data()), shader_code_.size());
	if (block.size() > 64 * 1024 * 1024)
	{
		os.setstate(std::ios::failbit);
		return;
	}
	auto const block_size = Native2LE(static_cast<uint32_t>(block.size()));
	os.write(reinterpret_cast<char const*>(&block_size), sizeof(block_size));
	os.write(block.data(), static_cast<std::streamsize>(block.size()));
}

void D3D12ShaderStageObject::CompileShader(const RenderEffect& effect, const RenderTechnique& tech,
	const RenderPass& pass, const std::array<uint32_t, ShaderStageNum>& shader_desc_ids)
{
	shader_code_.clear();
	shader_desc_ = {};
	cbuff_indices_.clear();
	is_validate_ = false;

	if (stage_ != ShaderStage::Vertex && stage_ != ShaderStage::Pixel)
	{
		return;
	}

	uint32_t const shader_desc_id = shader_desc_ids[std::to_underlying(stage_)];
	auto const& sd = effect.GetShaderDesc(shader_desc_id);
	if (sd.func_name.empty())
	{
		return;
	}

	entry_point_ = sd.func_name;
	shader_profile_ = std::string(GetShaderProfile(effect, shader_desc_id));
	if (shader_profile_.empty())
	{
		return;
	}

		auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		auto const& caps = re.DeviceCaps();
		d3d12::ShaderFormat const supported =
			re.Device() ? d3d12::GetShaderFormats(re.Device()) : d3d12::SHADERFORMAT_DXIL;

		// Same engine macros CompileToDXBC injects (KLAYGE_FLIPPING, SHADER_MODEL, …).
		std::string const max_sm_str = std::to_string(caps.max_shader_model.FullVersion());
		std::string const max_tex_array_str = std::to_string(caps.max_texture_array_length);
		std::string const max_tex_depth_str = std::to_string(caps.max_texture_depth);
		std::string const max_tex_units_str = std::to_string(static_cast<int>(caps.max_pixel_texture_units));
		std::string const flipping_str = std::to_string(re.RequiresFlipping() ? -1 : +1);
		std::string const render_to_tex_array_str = std::to_string(caps.render_to_texture_array_support ? 1 : 0);

		auto append_dxc_define = [](std::vector<std::wstring>& out, std::string const& name, std::string const& value) {
			std::wstring wname, wvalue;
			CommonWorker::Convert(wname, name);
			CommonWorker::Convert(wvalue, value);
			out.push_back(wname + L"=" + wvalue);
		};

		std::vector<std::wstring> defines;
		append_dxc_define(defines, "KLAYGE_Native", "1");
		// DXC-only: register spaces for d3d12:: D3D12 root signature. Must NOT be set for
		// FXC/DXBC reflection (vs_5_0 rejects spaceN and leaves cbuffers unbound).
		append_dxc_define(defines, "KLAYGE_NATIVE_GPU_BINDINGS", "1");
		append_dxc_define(defines, "KLAYGE_D3D11", "1");
		append_dxc_define(defines, "KLAYGE_FRAG_DEPTH", "1");
		append_dxc_define(defines, "KLAYGE_SHADER_MODEL", max_sm_str);
		append_dxc_define(defines, "KLAYGE_MAX_TEX_ARRAY_LEN", max_tex_array_str);
		append_dxc_define(defines, "KLAYGE_MAX_TEX_DEPTH", max_tex_depth_str);
		append_dxc_define(defines, "KLAYGE_MAX_TEX_UNITS", max_tex_units_str);
		append_dxc_define(defines, "KLAYGE_FLIPPING", flipping_str);
		append_dxc_define(defines, "KLAYGE_RENDER_TO_TEX_ARRAY", render_to_tex_array_str);
		if (!caps.fp_color_support)
		{
			append_dxc_define(defines, "KLAYGE_NO_FP_COLOR", "1");
		}
		if (caps.pack_to_rgba_required)
		{
			append_dxc_define(defines, "KLAYGE_PACK_TO_RGBA", "1");
		}
		if (caps.UavFormatSupport(EF_ABGR16F))
		{
			append_dxc_define(defines, "KLAYGE_TYPED_UAV_SUPPORT", "1");
		}
		if (caps.uavs_at_every_stage_support)
		{
			append_dxc_define(defines, "KLAYGE_UAVS_AT_EVERY_STAGE_SUPPORT", "1");
		}
		if (caps.explicit_multi_sample_support)
		{
			append_dxc_define(defines, "KLAYGE_EXPLICIT_MULTI_SAMPLE_SUPPORT", "1");
		}
		if (caps.vp_rt_index_at_every_stage_support)
		{
			append_dxc_define(defines, "KLAYGE_VP_RT_INDEX_AT_EVERY_STAGE_SUPPORT", "1");
		}
		if (stage_ == ShaderStage::Vertex)
		{
			append_dxc_define(defines, "KLAYGE_VERTEX_SHADER", "1");
		}
		else
		{
			append_dxc_define(defines, "KLAYGE_PIXEL_SHADER", "1");
		}
		for (uint32_t i = 0; i < tech.NumMacros(); ++i)
		{
			auto const& nv = tech.MacroByIndex(i);
			append_dxc_define(defines, nv.first, nv.second);
		}
		for (uint32_t i = 0; i < pass.NumMacros(); ++i)
		{
			auto const& nv = pass.MacroByIndex(i);
			append_dxc_define(defines, nv.first, nv.second);
		}

		// Reflect cbuffer layout via DXBC (same HLSL packing as D3D11), then prefer DXIL for GPU.
		// SKIP_OPTIMIZATION keeps unused predefined-cbuffer members in reflection so
		// PredefinedMesh/Model/Material/Camera CBuffer offsets can BindToCBuffer.
		std::vector<std::pair<char const*, char const*>> macros;
		macros.emplace_back("KLAYGE_Native", "1");
		macros.emplace_back("KLAYGE_D3D11", "1");
		macros.emplace_back("KLAYGE_FRAG_DEPTH", "1");

		uint32_t const reflect_flags =
			D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
		com_ptr<ID3D11ShaderReflection> reflection;
		auto dxbc = CompileToDXBC(stage_, effect, tech, pass, macros, entry_point_.c_str(), StageProfileDxbc(stage_),
			reflect_flags, reflection.put_void(), true);
		FillShaderDescFromReflection(reflection.get());
		FillCBufferIndices(effect);
		if (shader_desc_.cb_desc.empty())
		{
			LogWarn() << "[D3D12] No cbuffer reflection for " << entry_point_
					  << " (DXBC empty or D3DReflect failed); BindToCBuffer will be skipped." << std::endl;
		}

			if (supported & d3d12::SHADERFORMAT_DXIL)
			{
				std::string err;
				std::string dxil_src = ApplyRegisterSpaces(effect.HLSLShaderText(), shader_desc_);
				if (stage_ == ShaderStage::Vertex)
				{
					dxil_src = RemapVertexInputSemantics(std::move(dxil_src), entry_point_);
				}
				shader_code_ = DxcLoader::Instance().Compile(dxil_src, entry_point_.c_str(),
					StageProfileDxil(stage_), defines, err);
				shader_format_ = d3d12::SHADERFORMAT_DXIL;
				if (shader_code_.empty())
				{
					LogError() << "[D3D12] DXC/DXIL compile failed for " << entry_point_ << ": " << err << std::endl;
				}
			}

		if (shader_code_.empty() && !dxbc.empty() && (supported & d3d12::SHADERFORMAT_DXBC))
		{
			shader_code_ = std::move(dxbc);
			shader_format_ = d3d12::SHADERFORMAT_DXBC;
			LogWarn() << "[D3D12] Falling back to DXBC for " << entry_point_ << std::endl;
		}

	is_validate_ = !shader_code_.empty();
}

void D3D12ShaderStageObject::CreateHwShader(const RenderEffect& effect,
	[[maybe_unused]] const std::array<uint32_t, ShaderStageNum>& shader_desc_ids)
{
	ReleaseGpuShader();
	hw_res_ready_ = false;
	if (!is_validate_ || shader_code_.empty())
	{
		return;
	}

	auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	d3d12::Device* device = re.Device();
	if (!device)
	{
		return;
	}

	if (cbuff_indices_.empty() && !shader_desc_.cb_desc.empty())
	{
		FillCBufferIndices(effect);
	}

	d3d12::ShaderCreateInfo info{};
	info.code_size = shader_code_.size();
	info.code = shader_code_.data();
	info.entrypoint = entry_point_.c_str();
	info.format = shader_format_;
	info.stage = ToGpuStage(stage_);
	info.num_samplers = shader_desc_.num_samplers;
	info.num_storage_textures = 0;
	info.num_storage_buffers = 0;
	info.num_uniform_buffers = shader_desc_.num_uniform_buffers;
	if (info.num_uniform_buffers == 0 && !shader_desc_.cb_desc.empty())
	{
		info.num_uniform_buffers = static_cast<uint32_t>(shader_desc_.cb_desc.size());
	}

	gpu_shader_ = d3d12::CreateShader(device, &info);
	if (!gpu_shader_)
	{
		LogError() << "[D3D12] d3d12::CreateShader failed: " << d3d12::LastError() << std::endl;
		return;
	}
	gpu_device_ = device;
	device_lifetime_ = re.DeviceLifetime();
	hw_res_ready_ = true;
}

std::string_view D3D12ShaderStageObject::GetShaderProfile(RenderEffect const& effect, uint32_t shader_desc_id) const
{
	std::string_view profile = effect.GetShaderDesc(shader_desc_id).profile;
	if (profile.empty() || profile == "auto")
	{
		return StageProfileDxil(stage_);
	}
	if (profile.find("vs_") == 0)
	{
		return StageProfileDxil(ShaderStage::Vertex);
	}
	if (profile.find("ps_") == 0)
	{
		return StageProfileDxil(ShaderStage::Pixel);
	}
	return profile;
}

D3D12ShaderObject::D3D12ShaderObject() = default;

D3D12ShaderObject::D3D12ShaderObject(std::shared_ptr<Immutable> immutable) : ShaderObject(std::move(immutable))
{
}

ShaderObjectPtr D3D12ShaderObject::Clone([[maybe_unused]] RenderEffect& dst_effect)
{
	auto ret = MakeSharedPtr<D3D12ShaderObject>(immutable_);
	ret->hw_res_ready_ = hw_res_ready_;
	ret->vs_ = vs_;
	ret->ps_ = ps_;
	return ret;
}

void D3D12ShaderObject::Bind([[maybe_unused]] const RenderEffect& effect)
{
	vs_ = nullptr;
	ps_ = nullptr;
	auto const& vs_stage = Stage(ShaderStage::Vertex);
	auto const& ps_stage = Stage(ShaderStage::Pixel);
	if (vs_stage)
	{
		vs_ = checked_cast<D3D12ShaderStageObject&>(*vs_stage).GpuShader();
	}
	if (ps_stage)
	{
		ps_ = checked_cast<D3D12ShaderStageObject&>(*ps_stage).GpuShader();
	}
}

void D3D12ShaderObject::Unbind()
{
	vs_ = nullptr;
	ps_ = nullptr;
}

void D3D12ShaderObject::DoLinkShaders(RenderEffect& effect)
{
	for (auto stage : {ShaderStage::Vertex, ShaderStage::Pixel})
	{
		auto const& stage_obj = Stage(stage);
		if (!stage_obj)
		{
			continue;
		}

		auto const& native_stage = checked_cast<D3D12ShaderStageObject const&>(*stage_obj);
		auto const& cbuff_indices = native_stage.CBufferIndices();
		if (cbuff_indices.empty())
		{
			continue;
		}

		auto const& shader_desc = native_stage.GetShaderDesc();
		for (size_t i = 0; i < cbuff_indices.size(); ++i)
		{
			if (cbuff_indices[i] == 0xFF)
			{
				continue;
			}

			auto* cbuff = effect.CBufferByIndex(cbuff_indices[i]);
				cbuff->Resize(shader_desc.cb_desc[i].size);
				auto const& var_descs = shader_desc.cb_desc[i].var_desc;
				if (cbuff->NumParameters() != var_descs.size())
				{
					LogWarn() << "[D3D12] CBuffer '" << shader_desc.cb_desc[i].name
							  << "' param count mismatch: effect=" << cbuff->NumParameters()
							  << " reflection=" << var_descs.size()
							  << " (binding by name)" << std::endl;
				}
				for (uint32_t j = 0; j < cbuff->NumParameters(); ++j)
				{
					RenderEffectParameter* param = effect.ParameterByIndex(cbuff->ParameterIndex(j));
					// Prefer name match so effect/reflection order can diverge safely.
					uint32_t var_index = j;
					if (j < var_descs.size() && param->Name() != var_descs[j].name)
					{
						var_index = static_cast<uint32_t>(-1);
						for (uint32_t v = 0; v < var_descs.size(); ++v)
						{
							if (param->Name() == var_descs[v].name)
							{
								var_index = v;
								break;
							}
						}
						if (var_index == static_cast<uint32_t>(-1))
						{
							LogError() << "[D3D12] CBuffer var not in reflection: " << param->Name() << std::endl;
							continue;
						}
					}
					else if (j >= var_descs.size())
					{
						continue;
					}

					auto const& vd = var_descs[var_index];
					uint32_t stride;
					if (param->Type() == REDT_struct)
					{
						stride = 1;
					}
					else if (vd.elements > 0)
					{
						stride = (param->Type() != REDT_float4x4) ? 16u : 64u;
					}
					else
					{
						stride = (param->Type() != REDT_float4x4) ? 4u : 16u;
					}
					param->BindToCBuffer(effect, cbuff_indices[i], vd.start_offset, stride);
				}
		}
	}

	hw_res_ready_ = true;
	for (auto stage : {ShaderStage::Vertex, ShaderStage::Pixel})
	{
		auto const& s = immutable_->shader_stages_[std::to_underlying(stage)];
		if (s && !s->HWResourceReady())
		{
			hw_res_ready_ = false;
			break;
		}
	}
}

} // namespace RenderWorker
