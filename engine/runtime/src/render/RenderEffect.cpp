// RenderEffect.cpp
//////////////////////////////////////////////////////////////////////////////////

#include <base/ZEngine.h>
#include <base/ResLoader.h>

#include <render/RenderFactory.h>
#include <render/RenderStateObject.h>
#include <render/RenderView.h>
#include <render/ShaderObject.h>
#include <render/Texture.h>

#include <charconv>
#include <cstring>
#include <fstream>
#include <mutex>

#include <render/RenderEffect.h>
#include <render/EffectDesc.h>
#include <stdexcept>
#include <render/BuiltInEffects.h>
#include "RenderEffectInternal.h"

#include <common/Log.h>

namespace
{
	using namespace CommonWorker;
	using namespace RenderWorker;

	uint32_t const KFX_VERSION = 0x0151;
}

namespace RenderWorker
{
using namespace CommonWorker;
using namespace detail;

	RenderEffectAnnotation::RenderEffectAnnotation() = default;
	RenderEffectAnnotation::RenderEffectAnnotation(RenderEffectAnnotation&& rhs) noexcept = default;
	RenderEffectAnnotation& RenderEffectAnnotation::operator=(RenderEffectAnnotation&& rhs) noexcept = default;

	void RenderEffectAnnotation::StreamIn(RenderEffect const& effect, ResIdentifier& res)
	{
		res.read(&type_, sizeof(type_));
		type_ = LE2Native(type_);
		name_ = ReadShortString(res);
		var_ = StreamInVariable(effect, res, type_, 0);
	}

#if ZENGINE_IS_DEV_PLATFORM
	void RenderEffectAnnotation::StreamOut(std::ostream& os) const
	{
		uint32_t t = Native2LE(type_);
		os.write(reinterpret_cast<char const *>(&t), sizeof(t));
		WriteShortString(os, name_);
		StreamOutVariable(os, *var_);
	}
#endif

	RenderEffectStructType::RenderEffectStructType() = default;
	RenderEffectStructType::RenderEffectStructType(RenderEffectStructType&& rhs) noexcept = default;
	RenderEffectStructType& RenderEffectStructType::operator = (RenderEffectStructType && rhs) noexcept = default;

	void RenderEffectStructType::StreamIn(ResIdentifier& res)
	{
		name_ = ReadShortString(res);
		name_hash_ = HashValue(name_);

		uint8_t len;
		res.read(reinterpret_cast<char*>(&len), sizeof(len));

		members_.resize(len);
		for (uint32_t i = 0; i < len; ++i)
		{
			uint8_t type;
			res.read(reinterpret_cast<char*>(&type), sizeof(type));
			members_[i].type = static_cast<RenderEffectDataType>(type);

			if (type == REDT_struct)
			{
				members_[i].type_name = ReadShortString(res);
			}

			members_[i].name = ReadShortString(res);
			members_[i].array_size = MakeUniquePtr<std::string>(ReadShortString(res));
		}
	}

#if ZENGINE_IS_DEV_PLATFORM
	void RenderEffectStructType::StreamOut(std::ostream& os) const
	{
		WriteShortString(os, name_);

		uint8_t len = static_cast<uint8_t>(members_.size());
		os.write(reinterpret_cast<char const*>(&len), sizeof(len));

		for (uint32_t i = 0; i < len; ++i)
		{
			uint8_t const type = static_cast<uint8_t>(members_[i].type);
			os.write(reinterpret_cast<char const*>(&type), sizeof(type));

			if (type == REDT_struct)
			{
				WriteShortString(os, members_[i].type_name);
			}

			WriteShortString(os, members_[i].name);
			WriteShortString(os, *members_[i].array_size);
		}
	}
#endif

	uint32_t RenderEffectStructType::NumMembers() const noexcept
	{
		return static_cast<uint8_t>(members_.size());
	}

	RenderEffectDataType RenderEffectStructType::MemberType(uint32_t index) const noexcept
	{
		COMMON_ASSERT(index < this->NumMembers());
		return members_[index].type;
	}

	std::string const& RenderEffectStructType::MemberTypeName(uint32_t index) const noexcept
	{
		COMMON_ASSERT(index < this->NumMembers());
		return members_[index].type_name;
	}

	std::string const& RenderEffectStructType::MemberName(uint32_t index) const noexcept
	{
		COMMON_ASSERT(index < this->NumMembers());
		return members_[index].name;
	}

	std::string const* RenderEffectStructType::MemberArraySize(uint32_t index) const noexcept
	{
		COMMON_ASSERT(index < this->NumMembers());
		return members_[index].array_size.get();
	}

	RenderEffect::Immutable::Immutable() = default;
	RenderEffect::RenderEffect() = default;

#if ZENGINE_IS_DEV_PLATFORM
	void RenderEffect::CompileShaders()
	{
		if (immutable_->need_compile)
		{
			uint32_t tech_index = 0;
			for (auto& tech : immutable_->techniques)
			{
				tech.CompileShaders(*this, tech_index);
				++tech_index;
			}

            if (immutable_->description_based)
            {
                for (auto const& shader : shader_objs_)
                    for (uint32_t stage = 0; stage < ShaderStageNum; ++stage)
                        if (auto const& object = shader->Stage(static_cast<ShaderStage>(stage)); object && !object->Validate())
                            throw std::runtime_error("EffectDesc shader compilation failed: " + immutable_->res_name);
            }
			std::ofstream ofs(immutable_->kfx_name.c_str(), std::ios_base::binary | std::ios_base::out);
            if (immutable_->description_based)
            {
                auto const magic = Native2LE(detail::EffectDescCacheMagic);
                auto const description_hash = Native2LE(immutable_->description_hash);
                auto const source_hash = Native2LE(immutable_->source_hash);
                ofs.write(reinterpret_cast<char const*>(&magic), sizeof(magic));
                ofs.write(reinterpret_cast<char const*>(&description_hash), sizeof(description_hash));
                ofs.write(reinterpret_cast<char const*>(&source_hash), sizeof(source_hash));
            }
			this->StreamOut(ofs);
			LogInfo() << "Wrote kfx " << immutable_->kfx_name << std::endl;
		}
	}
#endif

	void RenderEffect::CreateHwShaders()
	{
		uint32_t tech_index = 0;
		for (auto& tech : immutable_->techniques)
		{
			tech.CreateHwShaders(*this, tech_index);
			++tech_index;
		}
	}

	RenderEffectPtr RenderEffect::Clone()
	{
		RenderEffectPtr ret = MakeSharedPtr<RenderEffect>();
		this->CloneInPlace(*ret);
		return ret;
	}

	void RenderEffect::CloneInPlace(RenderEffect& dst_effect)
	{
		dst_effect.immutable_ = immutable_;

		dst_effect.params_.resize(params_.size());
		for (size_t i = 0; i < params_.size(); ++i)
		{
			dst_effect.params_[i] = params_[i].Clone();
		}

		dst_effect.cbuffers_.resize(cbuffers_.size());
		for (size_t i = 0; i < cbuffers_.size(); ++i)
		{
			dst_effect.cbuffers_[i] = cbuffers_[i]->Clone(dst_effect);
		}

		dst_effect.shader_objs_.resize(shader_objs_.size());
		for (size_t i = 0; i < shader_objs_.size(); ++i)
		{
			if (shader_objs_[i]->HWResourceReady())
			{
				dst_effect.shader_objs_[i] = shader_objs_[i]->Clone(dst_effect);
			}
		}
	}

	void RenderEffect::Reclone(RenderEffect& dst_effect)
	{
		for (size_t i = 0; i < cbuffers_.size(); ++i)
		{
			cbuffers_[i]->Reclone(*dst_effect.cbuffers_[i], dst_effect);
		}

		for (size_t i = 0; i < shader_objs_.size(); ++i)
		{
			if (shader_objs_[i]->HWResourceReady())
			{
				dst_effect.shader_objs_[i] = shader_objs_[i]->Clone(dst_effect);
			}
		}
	}

	bool RenderEffect::HWResourceReady() const noexcept
	{
		if (!hw_res_ready_)
		{
			hw_res_ready_ = true;
			for (uint32_t i = 0; i < this->NumTechniques(); ++i)
			{
				hw_res_ready_ &= this->TechniqueByIndex(i)->HWResourceReady(*this);
			}
		}

		return hw_res_ready_;
	}

	RenderEffectParameter* RenderEffect::ParameterBySemantic(std::string_view semantic) noexcept
	{
		size_t const semantic_hash = HashValue(std::move(semantic));
		for (auto& param : params_)
		{
			if (semantic_hash == param.SemanticHash())
			{
				return &param;
			}
		}
		return nullptr;
	}

	RenderEffectParameter const* RenderEffect::ParameterBySemantic(std::string_view semantic) const noexcept
	{
		size_t const semantic_hash = HashValue(std::move(semantic));
		for (auto const& param : params_)
		{
			if (semantic_hash == param.SemanticHash())
			{
				return &param;
			}
		}
		return nullptr;
	}

	RenderEffectParameter* RenderEffect::ParameterByName(std::string_view name) noexcept
	{
		size_t const name_hash = HashValue(std::move(name));
		for (auto& param : params_)
		{
			if (name_hash == param.NameHash())
			{
				return &param;
			}
		}
		return nullptr;
	}

	RenderEffectParameter const* RenderEffect::ParameterByName(std::string_view name) const noexcept
	{
		size_t const name_hash = HashValue(std::move(name));
		for (auto const& param : params_)
		{
			if (name_hash == param.NameHash())
			{
				return &param;
			}
		}
		return nullptr;
	}

	RenderEffectParameter* RenderEffect::ParameterByIndex(uint32_t n) noexcept
	{
		COMMON_ASSERT(n < this->NumParameters());
		return &params_[n];
	}

	RenderEffectParameter const* RenderEffect::ParameterByIndex(uint32_t n) const noexcept
	{
		COMMON_ASSERT(n < this->NumParameters());
		return &params_[n];
	}

	RenderEffectConstantBuffer* RenderEffect::CBufferByName(std::string_view name) const noexcept
	{
		uint32_t index = this->FindCBuffer(name);
		if (index != static_cast<uint32_t>(-1))
		{
			return this->CBufferByIndex(index);
		}
		else
		{
			return nullptr;
		}
	}

	RenderEffectConstantBuffer* RenderEffect::CBufferByIndex(uint32_t index) const noexcept
	{
		COMMON_ASSERT(index < this->NumCBuffers());
		return cbuffers_[index].get();
	}

	uint32_t RenderEffect::FindCBuffer(std::string_view name) const noexcept
	{
		size_t const name_hash = HashValue(std::move(name));
		for (uint32_t i = 0; i < cbuffers_.size(); ++i)
		{
			if (name_hash == cbuffers_[i]->NameHash())
			{
				return i;
			}
		}
		return static_cast<uint32_t>(-1);
	}

	void RenderEffect::BindCBufferByName(std::string_view name, RenderEffectConstantBufferPtr const& cbuff) noexcept
	{
		size_t const name_hash = HashValue(std::move(name));
		for (auto& cbuffer : cbuffers_)
		{
			if (name_hash == cbuffer->NameHash())
			{
				cbuffer = cbuff;
			}
		}
	}

	void RenderEffect::BindCBufferByIndex(uint32_t index, RenderEffectConstantBufferPtr const& cbuff) noexcept
	{
		COMMON_ASSERT(index < this->NumCBuffers());
		cbuffers_[index] = cbuff;
	}



	RenderEffectStructType* RenderEffect::StructTypeByName(std::string_view name) const noexcept
	{
		size_t const name_hash = HashValue(std::move(name));
		for (auto& struct_type : immutable_->struct_types)
		{
			if (name_hash == struct_type.NameHash())
			{
				return &struct_type;
			}
		}
		return nullptr;
	}

	RenderEffectStructType* RenderEffect::StructTypeByIndex(uint32_t index) const noexcept
	{
		COMMON_ASSERT(index < this->NumStructTypes());
		return &immutable_->struct_types[index];
	}

	uint32_t RenderEffect::NumTechniques() const noexcept
	{
		return static_cast<uint32_t>(immutable_->techniques.size());
	}

	RenderTechnique* RenderEffect::TechniqueByName(std::string_view name) const noexcept
	{
		size_t const name_hash = HashValue(std::move(name));
		for (auto& tech : immutable_->techniques)
		{
			if (name_hash == tech.NameHash())
			{
				return &tech;
			}
		}
		return nullptr;
	}

	RenderTechnique* RenderEffect::TechniqueByIndex(uint32_t n) const noexcept
	{
		COMMON_ASSERT(n < this->NumTechniques());
		return &immutable_->techniques[n];
	}

	RenderShaderFragment const& RenderEffect::ShaderFragmentByIndex(uint32_t n) const noexcept
	{
		COMMON_ASSERT(n < this->NumShaderFragments());
		return immutable_->shader_frags[n];
	}

	uint32_t RenderEffect::AddShaderDesc(ShaderDesc const & sd)
	{
		for (uint32_t i = 0; i < immutable_->shader_descs.size(); ++i)
		{
			if (immutable_->shader_descs[i] == sd)
			{
				return i;
			}
		}

		uint32_t id = static_cast<uint32_t>(immutable_->shader_descs.size());
		immutable_->shader_descs.push_back(sd);
		return id;
	}

	ShaderDesc& RenderEffect::GetShaderDesc(uint32_t id) noexcept
	{
		COMMON_ASSERT(id < immutable_->shader_descs.size());
		return immutable_->shader_descs[id];
	}

	ShaderDesc const& RenderEffect::GetShaderDesc(uint32_t id) const noexcept
	{
		COMMON_ASSERT(id < immutable_->shader_descs.size());
		return immutable_->shader_descs[id];
	}

	std::pair<std::string, std::string> const& RenderEffect::MacroByIndex(uint32_t n) const noexcept
	{
		COMMON_ASSERT(n < this->NumMacros());
		return immutable_->macros[n];
	}

	uint32_t RenderEffect::AddShaderObject()
	{
		uint32_t index = static_cast<uint32_t>(shader_objs_.size());
		shader_objs_.push_back(Context::Instance().RenderFactoryInstance().MakeShaderObject());
		return index;
	}

	ShaderObjectPtr const& RenderEffect::ShaderObjectByIndex(uint32_t n) const noexcept
	{
		COMMON_ASSERT(n < shader_objs_.size());
		return shader_objs_[n];
	}

	bool RenderEffect::StreamIn(ResIdentifier& source)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		bool ret = false;

		uint32_t fourcc;
		source.read(&fourcc, sizeof(fourcc));
		fourcc = LE2Native(fourcc);

		uint32_t ver;
		source.read(&ver, sizeof(ver));
		ver = LE2Native(ver);

		if ((MakeFourCC<'K', 'F', 'X', ' '>::value == fourcc) && (KFX_VERSION == ver))
		{
			uint32_t shader_fourcc;
			source.read(&shader_fourcc, sizeof(shader_fourcc));
			shader_fourcc = LE2Native(shader_fourcc);

			uint32_t shader_ver;
			source.read(&shader_ver, sizeof(shader_ver));
			shader_ver = LE2Native(shader_ver);

			uint8_t shader_platform_name_len;
			source.read(&shader_platform_name_len, sizeof(shader_platform_name_len));
			std::string shader_platform_name(shader_platform_name_len, 0);
			source.read(&shader_platform_name[0], shader_platform_name_len);

			if ((re.NativeShaderFourCC() == shader_fourcc) && (re.NativeShaderVersion() == shader_ver)
				&& (re.NativeShaderPlatformName() == shader_platform_name))
			{
				uint64_t timestamp;
				source.read(&timestamp, sizeof(timestamp));
#if ZENGINE_IS_DEV_PLATFORM
				timestamp = LE2Native(timestamp);
				if (immutable_->timestamp <= timestamp)
#endif
				{
					immutable_->shader_descs.resize(1);

					{
						uint16_t num_macros;
						source.read(&num_macros, sizeof(num_macros));
						num_macros = LE2Native(num_macros);

						immutable_->macros.resize(num_macros);
						for (auto& macro : immutable_->macros)
						{
							std::string name = ReadShortString(source);
							std::string value = ReadShortString(source);
							macro = {std::move(name), std::move(value)};
						}
					}
					{
						uint16_t num_structs;
						source.read(&num_structs, sizeof(num_structs));
						num_structs = LE2Native(num_structs);

						immutable_->struct_types.resize(num_structs);
						for (auto& struct_type : immutable_->struct_types)
						{
							struct_type.StreamIn(source);
						}
					}
					{
						uint16_t num_cbufs;
						source.read(&num_cbufs, sizeof(num_cbufs));
						num_cbufs = LE2Native(num_cbufs);
						cbuffers_.resize(num_cbufs);
						for (auto& cbuff : cbuffers_)
						{
							cbuff = MakeSharedPtr<RenderEffectConstantBuffer>(*this);
							cbuff->StreamIn(source);
						}
					}
					{
						uint16_t num_params;
						source.read(&num_params, sizeof(num_params));
						num_params = LE2Native(num_params);
						params_.resize(num_params);
						for (auto& param : params_)
						{
							param.StreamIn(*this, source);
						}
					}
					{
						uint8_t num_shader_graph_nodes;
						source.read(&num_shader_graph_nodes, sizeof(num_shader_graph_nodes));
						immutable_->shader_graph_nodes.resize(num_shader_graph_nodes);
						for (auto& node : immutable_->shader_graph_nodes)
						{
							node.StreamIn(source);
						}
					}
					{
						uint16_t num_shader_frags;
						source.read(&num_shader_frags, sizeof(num_shader_frags));
						num_shader_frags = LE2Native(num_shader_frags);
						immutable_->shader_frags.resize(num_shader_frags);
						for (auto& frag : immutable_->shader_frags)
						{
							frag.StreamIn(source);
						}
					}

					{
						uint16_t num_shader_descs;
						source.read(&num_shader_descs, sizeof(num_shader_descs));
						num_shader_descs = LE2Native(num_shader_descs);
						immutable_->shader_descs.resize(num_shader_descs + 1);
						for (uint32_t i = 1; i <= num_shader_descs; ++ i)
						{
							immutable_->shader_descs[i].profile = ReadShortString(source);
							immutable_->shader_descs[i].func_name = ReadShortString(source);
							source.read(&immutable_->shader_descs[i].macros_hash, sizeof(immutable_->shader_descs[i].macros_hash));

							source.read(&immutable_->shader_descs[i].tech_pass_type, sizeof(immutable_->shader_descs[i].tech_pass_type));
							immutable_->shader_descs[i].tech_pass_type = LE2Native(immutable_->shader_descs[i].tech_pass_type);

							uint8_t len;
							source.read(&len, sizeof(len));
							if (len > 0)
							{
								immutable_->shader_descs[i].so_decl.resize(len);
								source.read(
									&immutable_->shader_descs[i].so_decl[0], len * sizeof(immutable_->shader_descs[i].so_decl[0]));
								for (uint32_t j = 0; j < len; ++ j)
								{
									immutable_->shader_descs[i].so_decl[j].usage = LE2Native(immutable_->shader_descs[i].so_decl[j].usage);
								}
							}
						}
					}

					ret = true;
					{
						uint16_t num_techs;
						source.read(&num_techs, sizeof(num_techs));
						num_techs = LE2Native(num_techs);
						immutable_->techniques.resize(num_techs);
						for (uint32_t i = 0; i < num_techs; ++ i)
						{
							ret &= immutable_->techniques[i].StreamIn(*this, source, i);
						}
					}
				}
			}
		}

		return ret;
	}

#if ZENGINE_IS_DEV_PLATFORM
	void RenderEffect::StreamOut(std::ostream& os) const
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		uint32_t fourcc = Native2LE(MakeFourCC<'K', 'F', 'X', ' '>::value);
		os.write(reinterpret_cast<char const *>(&fourcc), sizeof(fourcc));

		uint32_t ver = Native2LE(KFX_VERSION);
		os.write(reinterpret_cast<char const *>(&ver), sizeof(ver));

		uint32_t shader_fourcc = Native2LE(re.NativeShaderFourCC());
		os.write(reinterpret_cast<char const *>(&shader_fourcc), sizeof(shader_fourcc));

		uint32_t shader_ver = Native2LE(re.NativeShaderVersion());
		os.write(reinterpret_cast<char const *>(&shader_ver), sizeof(shader_ver));

		uint8_t shader_platform_name_len = static_cast<uint8_t>(re.NativeShaderPlatformName().size());
		os.write(reinterpret_cast<char const *>(&shader_platform_name_len), sizeof(shader_platform_name_len));
		os.write(&re.NativeShaderPlatformName()[0], shader_platform_name_len);

		uint64_t timestamp = Native2LE(immutable_->timestamp);
		os.write(reinterpret_cast<char const *>(&timestamp), sizeof(timestamp));

		{
			uint16_t num_macros = Native2LE(static_cast<uint16_t>(immutable_->macros.size()));
			os.write(reinterpret_cast<char const *>(&num_macros), sizeof(num_macros));

			for (auto const& macro : immutable_->macros)
			{
				WriteShortString(os, macro.first);
				WriteShortString(os, macro.second);
			}
		}
		{
			uint16_t num_structs = Native2LE(static_cast<uint16_t>(immutable_->struct_types.size()));
			os.write(reinterpret_cast<char const*>(&num_structs), sizeof(num_structs));
			for (auto const& struct_type : immutable_->struct_types)
			{
				struct_type.StreamOut(os);
			}
		}
		{
			uint16_t num_cbufs = Native2LE(static_cast<uint16_t>(cbuffers_.size()));
			os.write(reinterpret_cast<char const *>(&num_cbufs), sizeof(num_cbufs));
			for (auto const& cbuff : cbuffers_)
			{
				cbuff->StreamOut(os);
			}
		}
		{
			uint16_t num_params = Native2LE(static_cast<uint16_t>(params_.size()));
			os.write(reinterpret_cast<char const *>(&num_params), sizeof(num_params));
			for (auto const& param : params_)
			{
				param.StreamOut(os);
			}
		}
		{
			uint8_t num_shader_graph_nodes = static_cast<uint8_t>(immutable_->shader_graph_nodes.size());
			os.write(reinterpret_cast<char const *>(&num_shader_graph_nodes), sizeof(num_shader_graph_nodes));
			for (auto const& node : immutable_->shader_graph_nodes)
			{
				node.StreamOut(os);
			}
		}
		{
			uint16_t num_shader_frags = Native2LE(static_cast<uint16_t>(immutable_->shader_frags.size()));
			os.write(reinterpret_cast<char const *>(&num_shader_frags), sizeof(num_shader_frags));
			for (auto const& frag : immutable_->shader_frags)
			{
				frag.StreamOut(os);
			}
		}
		{
			uint16_t num_shader_descs = Native2LE(static_cast<uint16_t>(immutable_->shader_descs.size() - 1));
			os.write(reinterpret_cast<char const *>(&num_shader_descs), sizeof(num_shader_descs));
			for (uint32_t i = 1; i < immutable_->shader_descs.size(); ++i)
			{
				WriteShortString(os, immutable_->shader_descs[i].profile);
				WriteShortString(os, immutable_->shader_descs[i].func_name);

				uint64_t tmp64 = Native2LE(immutable_->shader_descs[i].macros_hash);
				os.write(reinterpret_cast<char const *>(&tmp64), sizeof(tmp64));

				uint32_t tmp32 = Native2LE(immutable_->shader_descs[i].tech_pass_type);
				os.write(reinterpret_cast<char const *>(&tmp32), sizeof(tmp32));

				uint8_t len = static_cast<uint8_t>(immutable_->shader_descs[i].so_decl.size());
				os.write(reinterpret_cast<char const *>(&len), sizeof(len));
				for (uint32_t j = 0; j < len; ++ j)
				{
					ShaderDesc::StreamOutputDecl so_decl = immutable_->shader_descs[i].so_decl[j];
					so_decl.usage = Native2LE(so_decl.usage);
					os.write(reinterpret_cast<char const *>(&so_decl), sizeof(so_decl));
				}
			}
		}

		{
			uint16_t num_techs = Native2LE(static_cast<uint16_t>(immutable_->techniques.size()));
			os.write(reinterpret_cast<char const *>(&num_techs), sizeof(num_techs));
			for (uint32_t i = 0; i < immutable_->techniques.size(); ++i)
			{
				immutable_->techniques[i].StreamOut(*this, os, i);
			}
		}
	}
#endif

#if ZENGINE_IS_DEV_PLATFORM
	void RenderEffect::GenHLSLShaderText()
	{
		std::string& str = immutable_->hlsl_shader;
		str.clear();

		str += "#define SHADER_MODEL(major, minor) ((major) * 4 + (minor))\n\n";

		for (auto const& macro : immutable_->macros)
		{
			str += "#define " + macro.first + " " + macro.second + "\n";
		}
		str += '\n';

		// D3D12 root signature: VS uniforms (b[n], space1),
		// PS uniforms (b[n], space3); sampled resources (t/s[n], space0 VS / space2 PS).
		// Only enabled for DXC/DXIL (KLAYGE_NATIVE_GPU_BINDINGS). FXC vs_5_0 reflection
		// cannot parse register spaces, so BindToCBuffer uses classic space0 DXBC.
		str += "#if KLAYGE_NATIVE_GPU_BINDINGS\n";
		str += "#if KLAYGE_VERTEX_SHADER\n";
		str += "#define KLAYGE_NATIVE_CBV_SPACE space1\n";
		str += "#define KLAYGE_NATIVE_RES_SPACE space0\n";
		str += "#elif KLAYGE_PIXEL_SHADER\n";
		str += "#define KLAYGE_NATIVE_CBV_SPACE space3\n";
		str += "#define KLAYGE_NATIVE_RES_SPACE space2\n";
		str += "#elif KLAYGE_COMPUTE_SHADER\n";
		str += "#define KLAYGE_NATIVE_CBV_SPACE space1\n";
		str += "#define KLAYGE_NATIVE_RES_SPACE space0\n";
		str += "#else\n";
		str += "#define KLAYGE_NATIVE_CBV_SPACE space0\n";
		str += "#define KLAYGE_NATIVE_RES_SPACE space0\n";
		str += "#endif\n";
		str += "#endif\n\n";

		for (auto const& struct_type : immutable_->struct_types)
		{
			str += "struct " + struct_type.Name() + "\n";
			str += "{\n";
			for (uint32_t j = 0; j < struct_type.NumMembers(); ++j)
			{
				RenderEffectDataType member_type = struct_type.MemberType(j);
				if (member_type == REDT_struct)
				{
					str += "    ";
					str += struct_type.MemberTypeName(j);
				}
				else
				{
					str += std::string(TypeNameFromCode(member_type));
				}

				str += " " + struct_type.MemberName(j);

				auto const* array_size_str = struct_type.MemberArraySize(j);
				if ((array_size_str != nullptr) && !array_size_str->empty())
				{
					str += "[" + *array_size_str + "]";
				}
				str += ";\n";
			}
			str += "};\n";
		}

		for (auto const& cbuff : cbuffers_)
		{
			str += "cbuffer " + cbuff->Name() + "\n";
			str += "{\n";

			for (uint32_t j = 0; j < cbuff->NumParameters(); ++j)
			{
				RenderEffectParameter const& param = *this->ParameterByIndex(cbuff->ParameterIndex(j));
				switch (param.Type())
				{
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
				case REDT_sampler:
				case REDT_buffer:
				case REDT_structured_buffer:
				case REDT_byte_address_buffer:
				case REDT_rw_buffer:
				case REDT_rw_structured_buffer:
				case REDT_rw_texture1D:
				case REDT_rw_texture2D:
				case REDT_rw_texture3D:
				case REDT_rw_texture1DArray:
				case REDT_rw_texture2DArray:
				case REDT_rw_byte_address_buffer:
				case REDT_append_structured_buffer:
				case REDT_consume_structured_buffer:
				case REDT_rasterizer_ordered_buffer:
				case REDT_rasterizer_ordered_byte_address_buffer:
				case REDT_rasterizer_ordered_structured_buffer:
				case REDT_rasterizer_ordered_texture1D:
				case REDT_rasterizer_ordered_texture1DArray:
				case REDT_rasterizer_ordered_texture2D:
				case REDT_rasterizer_ordered_texture2DArray:
				case REDT_rasterizer_ordered_texture3D:
					break;

				default:
					if (param.Type() == REDT_struct)
					{
						str += param.StructType()->Name();
					}
					else
					{
						str += std::string(TypeNameFromCode(param.Type()));
					}

					str += " " + param.Name();

					auto const* array_size_str = param.ArraySize();
					if ((array_size_str != nullptr) && !array_size_str->empty())
					{
						str += "[" + *array_size_str + "]";
					}
					str += ";\n";
					break;
				}
			}

			str += "};\n";
		}

		uint32_t native_srv_reg = 0;
		uint32_t native_samp_reg = 0;
		// Resource registers are injected later for DXC from FXC reflection bind points
		// (dense used slots). Assigning every effect resource a fixed register here made
		// DXIL need b2/tN while num_uniform_buffers/num_samplers only covered used counts.
		auto emit_native_resource = [&str](std::string const& decl, char /*reg_letter*/, uint32_t& /*reg_counter*/) {
			str += decl + ";\n";
		};
		(void)native_srv_reg;
		(void)native_samp_reg;

		for (auto const& param : params_)
		{
			std::string elem_type;
			switch (param.Type())
			{
			case REDT_texture1D:
			case REDT_texture2D:
			case REDT_texture2DMS:
			case REDT_texture3D:
			case REDT_textureCUBE:
			case REDT_texture1DArray:
			case REDT_texture2DArray:
			case REDT_texture2DMSArray:
			case REDT_textureCUBEArray:
			case REDT_buffer:
			case REDT_structured_buffer:
			case REDT_rw_buffer:
			case REDT_rw_structured_buffer:
			case REDT_rw_texture1D:
			case REDT_rw_texture2D:
			case REDT_rw_texture3D:
			case REDT_rw_texture1DArray:
			case REDT_rw_texture2DArray:
			case REDT_append_structured_buffer:
			case REDT_consume_structured_buffer:
			case REDT_rasterizer_ordered_buffer:
			case REDT_rasterizer_ordered_byte_address_buffer:
			case REDT_rasterizer_ordered_structured_buffer:
			case REDT_rasterizer_ordered_texture1D:
			case REDT_rasterizer_ordered_texture1DArray:
			case REDT_rasterizer_ordered_texture2D:
			case REDT_rasterizer_ordered_texture2DArray:
			case REDT_rasterizer_ordered_texture3D:
				param.Var().Value(elem_type);
				break;

			default:
				break;
			}

			std::string const & param_name = param.Name();
			switch (param.Type())
			{
			case REDT_texture1D:
				emit_native_resource("Texture1D<" + elem_type + "> " + param_name, 't', native_srv_reg);
				break;

			case REDT_texture2D:
				emit_native_resource("Texture2D<" + elem_type + "> " + param_name, 't', native_srv_reg);
				break;

			case REDT_texture2DMS:
				str += "#if KLAYGE_EXPLICIT_MULTI_SAMPLE_SUPPORT\n";
				emit_native_resource("Texture2DMS<" + elem_type + "> " + param_name, 't', native_srv_reg);
				str += "#endif\n";
				break;

			case REDT_texture3D:
				str += "#if KLAYGE_MAX_TEX_DEPTH <= 1\n";
				str += "Texture2D<" + elem_type + "> " + param_name + ";\n";
				str += "#else\n";
				str += "Texture3D<" + elem_type + "> " + param_name + ";\n";
				str += "#endif\n";
				break;

			case REDT_textureCUBE:
				emit_native_resource("TextureCube<" + elem_type + "> " + param_name, 't', native_srv_reg);
				break;

			case REDT_texture1DArray:
				str += "#if KLAYGE_MAX_TEX_ARRAY_LEN > 1\n";
				emit_native_resource("Texture1DArray<" + elem_type + "> " + param_name, 't', native_srv_reg);
				str += "#endif\n";
				break;

			case REDT_texture2DArray:
				str += "#if KLAYGE_MAX_TEX_ARRAY_LEN > 1\n";
				emit_native_resource("Texture2DArray<" + elem_type + "> " + param_name, 't', native_srv_reg);
				str += "#endif\n";
				break;

			case REDT_texture2DMSArray:
				str += "#if KLAYGE_MAX_TEX_ARRAY_LEN > 1\n";
				str += "#if KLAYGE_EXPLICIT_MULTI_SAMPLE_SUPPORT\n";
				emit_native_resource("Texture2DMSArray<" + elem_type + "> " + param_name, 't', native_srv_reg);
				str += "#endif\n";
				str += "#endif\n";
				break;

			case REDT_textureCUBEArray:
				str += "#if (KLAYGE_MAX_TEX_ARRAY_LEN > 1) && (KLAYGE_SHADER_MODEL >= SHADER_MODEL(4, 1))\n";
				emit_native_resource("TextureCubeArray<" + elem_type + "> " + param_name, 't', native_srv_reg);
				str += "#endif\n";
				break;

			case REDT_buffer:
				emit_native_resource("Buffer<" + elem_type + "> " + param_name, 't', native_srv_reg);
				break;

			case REDT_sampler:
				emit_native_resource("sampler " + param_name, 's', native_samp_reg);
				break;

			case REDT_structured_buffer:
				emit_native_resource("StructuredBuffer<" + elem_type + "> " + param_name, 't', native_srv_reg);
				break;

			case REDT_byte_address_buffer:
				emit_native_resource("ByteAddressBuffer " + param_name, 't', native_srv_reg);
				break;

			case REDT_rw_buffer:
				str += "#if KLAYGE_SHADER_MODEL >= SHADER_MODEL(5, 0)\n";
				str += "RWBuffer<" + elem_type + "> " + param_name + ";\n";
				str += "#endif\n";
				break;

			case REDT_rw_structured_buffer:
				str += "RWStructuredBuffer<" + elem_type + "> " + param_name + ";\n";
				break;

			case REDT_rw_texture1D:
				str += "#if KLAYGE_SHADER_MODEL >= SHADER_MODEL(5, 0)\n";
				str += "RWTexture1D<" + elem_type + "> " + param_name + ";\n";
				str += "#endif\n";
				break;

			case REDT_rw_texture2D:
				str += "#if KLAYGE_SHADER_MODEL >= SHADER_MODEL(5, 0)\n";
				str += "RWTexture2D<" + elem_type + "> " + param_name + ";\n";
				str += "#endif\n";
				break;

			case REDT_rw_texture3D:
				str += "#if KLAYGE_SHADER_MODEL >= SHADER_MODEL(5, 0)\n";
				str += "RWTexture3D<" + elem_type + "> " + param_name + ";\n";
				str += "#endif\n";
				break;

			case REDT_rw_texture1DArray:
				str += "#if KLAYGE_SHADER_MODEL >= SHADER_MODEL(5, 0)\n";
				str += "RWTexture1DArray<" + elem_type + "> " + param_name + ";\n";
				str += "#endif\n";
				break;

			case REDT_rw_texture2DArray:
				str += "#if KLAYGE_SHADER_MODEL >= SHADER_MODEL(5, 0)\n";
				str += "RWTexture2DArray<" + elem_type + "> " + param_name + ";\n";
				str += "#endif\n";
				break;

			case REDT_rw_byte_address_buffer:
				str += "RWByteAddressBuffer " + param_name + ";\n";
				break;

			case REDT_append_structured_buffer:
				str += "#if KLAYGE_SHADER_MODEL >= SHADER_MODEL(5, 0)\n";
				str += "AppendStructuredBuffer<" + elem_type + "> " + param_name + ";\n";
				str += "#endif\n";
				break;

			case REDT_consume_structured_buffer:
				str += "#if KLAYGE_SHADER_MODEL >= SHADER_MODEL(5, 0)\n";
				str += "ConsumeStructuredBuffer<" + elem_type + "> " + param_name + ";\n";
				str += "#endif\n";
				break;

			case REDT_rasterizer_ordered_buffer:
				str += "#if KLAYGE_SHADER_MODEL >= SHADER_MODEL(5, 1)\n";
				str += "RasterizerOrderedBuffer<" + elem_type + "> " + param_name + ";\n";
				str += "#endif\n";
				break;

			case REDT_rasterizer_ordered_byte_address_buffer:
				str += "#if KLAYGE_SHADER_MODEL >= SHADER_MODEL(5, 1)\n";
				str += "RasterizerOrderedByteAddressBuffer " + param_name + ";\n";
				str += "#endif\n";
				break;

			case REDT_rasterizer_ordered_structured_buffer:
				str += "#if KLAYGE_SHADER_MODEL >= SHADER_MODEL(5, 1)\n";
				str += "RasterizerOrderedStructuredBuffer<" + elem_type + "> " + param_name + ";\n";
				str += "#endif\n";
				break;

			case REDT_rasterizer_ordered_texture1D:
				str += "#if KLAYGE_SHADER_MODEL >= SHADER_MODEL(5, 1)\n";
				str += "RasterizerOrderedTexture1D<" + elem_type + "> " + param_name + ";\n";
				str += "#endif\n";
				break;

			case REDT_rasterizer_ordered_texture1DArray:
				str += "#if KLAYGE_SHADER_MODEL >= SHADER_MODEL(5, 1)\n";
				str += "RasterizerOrderedTexture1DArray<" + elem_type + "> " + param_name + ";\n";
				str += "#endif\n";
				break;

			case REDT_rasterizer_ordered_texture2D:
				str += "#if KLAYGE_SHADER_MODEL >= SHADER_MODEL(5, 1)\n";
				str += "RasterizerOrderedTexture2D<" + elem_type + "> " + param_name + ";\n";
				str += "#endif\n";
				break;

			case REDT_rasterizer_ordered_texture2DArray:
				str += "#if KLAYGE_SHADER_MODEL >= SHADER_MODEL(5, 1)\n";
				str += "RasterizerOrderedTexture2DArray<" + elem_type + "> " + param_name + ";\n";
				str += "#endif\n";
				break;

			case REDT_rasterizer_ordered_texture3D:
				str += "#if KLAYGE_SHADER_MODEL >= SHADER_MODEL(5, 1)\n";
				str += "RasterizerOrderedTexture3D<" + elem_type + "> " + param_name + ";\n";
				str += "#endif\n";
				break;

			default:
				break;
			}
		}

		if (!immutable_->shader_graph_nodes.empty())
		{
			for (auto const& node : immutable_->shader_graph_nodes)
			{
				str += node.GenDeclarationCode();
			}
			str += '\n';
		}

		for (auto const& frag : immutable_->shader_frags)
		{
			ShaderStage const shader_stage = frag.Stage();
			switch (shader_stage)
			{
			case ShaderStage::Vertex:
				str += "#if KLAYGE_VERTEX_SHADER\n";
				break;

			case ShaderStage::Pixel:
				str += "#if KLAYGE_PIXEL_SHADER\n";
				break;

			case ShaderStage::Geometry:
				str += "#if KLAYGE_GEOMETRY_SHADER\n";
				break;

			case ShaderStage::Compute:
				str += "#if KLAYGE_COMPUTE_SHADER\n";
				break;

			case ShaderStage::Hull:
				str += "#if KLAYGE_HULL_SHADER\n";
				break;

			case ShaderStage::Domain:
				str += "#if KLAYGE_DOMAIN_SHADER\n";
				break;

			case ShaderStage::NumStages:
				break;

			default:
				ZENGINE_UNREACHABLE("Invalid shader type");
			}
			ShaderModel const & ver = frag.Version();
			if ((ver.major_ver != 0) || (ver.minor_ver != 0))
			{
				str += std::format(
					"#if KLAYGE_SHADER_MODEL >= SHADER_MODEL({}, {})\n", static_cast<int>(ver.major_ver), static_cast<int>(ver.minor_ver));
			}

			str += frag.str() + "\n";

			if ((ver.major_ver != 0) || (ver.minor_ver != 0))
			{
				str += "#endif\n";
			}
			if (shader_stage != ShaderStage::NumStages)
			{
				str += "#endif\n";
			}
		}

		if (!immutable_->shader_graph_nodes.empty())
		{
			str += '\n';
			for (auto const& node : immutable_->shader_graph_nodes)
			{
				str += node.GenDefinitionCode();
			}
			str += '\n';
		}
	}
#endif

	RenderTechnique::RenderTechnique() = default;
	RenderTechnique::RenderTechnique(RenderTechnique&& rhs) noexcept = default;
	RenderTechnique& RenderTechnique::operator=(RenderTechnique&& rhs) noexcept = default;

#if ZENGINE_IS_DEV_PLATFORM
	void RenderTechnique::CompileShaders(RenderEffect& effect, uint32_t tech_index)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		const auto& caps = re.DeviceCaps();
		if (passes_.empty() || ver_ > caps.max_shader_model)
		{
			is_validate_ = false;
			return;
		}

		uint32_t pass_index = 0;
		for (auto& pass : passes_)
		{
			pass->CompileShaders(effect, tech_index, pass_index);
			++pass_index;
		}
	}
#endif

	void RenderTechnique::CreateHwShaders(RenderEffect& effect, uint32_t tech_index)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		const auto& caps = re.DeviceCaps();
		if (passes_.empty() || ver_ > caps.max_shader_model)
		{
			is_validate_ = false;
			return;
		}

		is_validate_ = true;

		has_discard_ = false;
		has_tessellation_ = false;

		uint32_t pass_index = 0;
		for (auto& pass : passes_)
		{
			pass->CreateHwShaders(effect, tech_index, pass_index);

			is_validate_ &= pass->Validate();

			auto const* shader_obj = pass->GetShaderObject(effect).get();
			if (auto const* ps_stage = shader_obj->Stage(ShaderStage::Pixel).get())
			{
				has_discard_ |= ps_stage->HasDiscard();
			}
			has_tessellation_ |= !!shader_obj->Stage(ShaderStage::Hull);

			++pass_index;
		}
	}

	bool RenderTechnique::StreamIn(RenderEffect& effect, ResIdentifier& res, uint32_t tech_index)
	{
		// Empty records represent backend-unsupported techniques, not corrupt caches.
		is_validate_ = false;
		has_discard_ = false;
		has_tessellation_ = false;
		name_ = ReadShortString(res);
		name_hash_ = HashValue(name_);

		res.read(&ver_, sizeof(ver_));

		uint8_t num_anno;
		res.read(&num_anno, sizeof(num_anno));
		if (num_anno > 0)
		{
			annotations_ = MakeSharedPtr<std::remove_reference<decltype(*annotations_)>::type>();
			annotations_->resize(num_anno);
			for (uint32_t i = 0; i < num_anno; ++ i)
			{
				RenderEffectAnnotationPtr annotation = MakeSharedPtr<RenderEffectAnnotation>();
				(*annotations_)[i] = annotation;

				annotation->StreamIn(effect, res);
			}
		}

		uint8_t num_macro;
		res.read(&num_macro, sizeof(num_macro));
		if (num_macro > 0)
		{
			macros_ = MakeSharedPtr<std::remove_reference<decltype(*macros_)>::type>();
			macros_->resize(num_macro);
			for (uint32_t i = 0; i < num_macro; ++ i)
			{
				std::string name = ReadShortString(res);
				std::string value = ReadShortString(res);
				(*macros_)[i] = {std::move(name), std::move(value)};
			}
		}

		res.read(&transparent_, sizeof(transparent_));
		res.read(&weight_, sizeof(weight_));
		weight_ = LE2Native(weight_);

		bool ret = true;
		uint8_t num_passes;
		res.read(&num_passes, sizeof(num_passes));
		passes_.resize(num_passes);
		for (uint32_t pass_index = 0; pass_index < num_passes; ++ pass_index)
		{
			RenderPassPtr pass = MakeSharedPtr<RenderPass>();
			passes_[pass_index] = pass;

			ret &= pass->StreamIn(effect, res, tech_index, pass_index);
		}

		return ret;
	}

#if ZENGINE_IS_DEV_PLATFORM
	void RenderTechnique::StreamOut(RenderEffect const & effect, std::ostream& os, uint32_t tech_index) const
	{
		WriteShortString(os, name_);

		os.write(reinterpret_cast<char const*>(&ver_), sizeof(ver_));

		uint8_t num_anno;
		if (annotations_)
		{
			num_anno = static_cast<uint8_t>(annotations_->size());
		}
		else
		{
			num_anno = 0;
		}
		os.write(reinterpret_cast<char const *>(&num_anno), sizeof(num_anno));
		for (uint32_t i = 0; i < num_anno; ++ i)
		{
			(*annotations_)[i]->StreamOut(os);
		}

		uint8_t num_macro;
		if (macros_)
		{
			num_macro = static_cast<uint8_t>(macros_->size());
		}
		else
		{
			num_macro = 0;
		}
		os.write(reinterpret_cast<char const *>(&num_macro), sizeof(num_macro));
		for (uint32_t i = 0; i < num_macro; ++ i)
		{
			WriteShortString(os, (*macros_)[i].first);
			WriteShortString(os, (*macros_)[i].second);
		}

		os.write(reinterpret_cast<char const *>(&transparent_), sizeof(transparent_));
		float w = Native2LE(weight_);
		os.write(reinterpret_cast<char const *>(&w), sizeof(w));

		uint8_t num_passes = static_cast<uint8_t>(passes_.size());
		os.write(reinterpret_cast<char const *>(&num_passes), sizeof(num_passes));
		for (uint32_t pass_index = 0; pass_index < num_passes; ++ pass_index)
		{
			passes_[pass_index]->StreamOut(effect, os, tech_index, pass_index);
		}
	}
#endif

	const RenderEffectAnnotation& RenderTechnique::Annotation(uint32_t n) const noexcept
	{
		COMMON_ASSERT(n < this->NumAnnotations());
		return *(*annotations_)[n];
	}

	std::pair<std::string, std::string> const& RenderTechnique::MacroByIndex(uint32_t n) const noexcept
	{
		COMMON_ASSERT(n < this->NumMacros());
		return (*macros_)[n];
	}

	RenderPass const& RenderTechnique::Pass(uint32_t n) const noexcept
	{
		COMMON_ASSERT(n < this->NumPasses());
		return *passes_[n];
	}

	bool RenderTechnique::HWResourceReady(RenderEffect const& effect) const noexcept
	{
		bool hw_res_ready = true;
		for (auto const& pass : passes_)
		{
			auto const* shader_obj = pass->GetShaderObject(effect).get();
			if (shader_obj)
			{
				hw_res_ready &= shader_obj->HWResourceReady();
			}
			else
			{
				hw_res_ready = false;
				break;
			}
		}

		return hw_res_ready;
	}

	RenderPass::RenderPass() = default;

#if ZENGINE_IS_DEV_PLATFORM
	void RenderPass::CompileShaders(RenderEffect& effect, uint32_t tech_index, uint32_t pass_index)
	{
		auto const & shader_obj = this->GetShaderObject(effect);
		for (uint32_t stage_index = 0; stage_index < ShaderStageNum; ++stage_index)
		{
			ShaderDesc const& sd = effect.GetShaderDesc(shader_desc_ids_[stage_index]);
			if (!sd.func_name.empty())
			{
				ShaderStage const stage = static_cast<ShaderStage>(stage_index);
				if (sd.tech_pass_type == (tech_index << 16) + (pass_index << 8) + stage_index)
				{
					auto const& stage_obj = shader_obj->Stage(stage);
					if (!stage_obj)
					{
						LogError() << "CompileShaders: missing ShaderStageObject for stage "
								   << stage_index << " in pass " << name_ << std::endl;
						continue;
					}
					auto const & tech = *effect.TechniqueByIndex(tech_index);
					stage_obj->CompileShader(effect, tech, *this, shader_desc_ids_);
				}
			}
		}
	}
#endif

	void RenderPass::CreateHwShaders(RenderEffect& effect, uint32_t tech_index, uint32_t pass_index)
	{
		auto const & shader_obj = this->GetShaderObject(effect);
		for (uint32_t stage_index = 0; stage_index < ShaderStageNum; ++stage_index)
		{
			ShaderDesc const& sd = effect.GetShaderDesc(shader_desc_ids_[stage_index]);
			if (!sd.func_name.empty())
			{
				ShaderStage const stage = static_cast<ShaderStage>(stage_index);
				if (sd.tech_pass_type == (tech_index << 16) + (pass_index << 8) + stage_index)
				{
					auto const& stage_obj = shader_obj->Stage(stage);
					if (!stage_obj)
					{
						LogError() << "CreateHwShaders: missing ShaderStageObject for stage "
								   << stage_index << " in pass " << name_ << std::endl;
						continue;
					}
					stage_obj->CreateHwShader(effect, shader_desc_ids_);
				}
			}
		}

		shader_obj->LinkShaders(effect);

		is_validate_ = shader_obj->Validate();
	}

	bool RenderPass::StreamIn(RenderEffect& effect, ResIdentifier& res, uint32_t tech_index, uint32_t pass_index)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		name_ = ReadShortString(res);
		name_hash_ = HashValue(name_);

		uint8_t num_anno;
		res.read(&num_anno, sizeof(num_anno));
		if (num_anno > 0)
		{
			annotations_ = MakeSharedPtr<std::remove_reference<decltype(*annotations_)>::type>();
			annotations_->resize(num_anno);
			for (uint32_t i = 0; i < num_anno; ++ i)
			{
				RenderEffectAnnotationPtr annotation = MakeSharedPtr<RenderEffectAnnotation>();
				(*annotations_)[i] = annotation;
				annotation->StreamIn(effect, res);
			}
		}

		uint8_t num_macro;
		res.read(&num_macro, sizeof(num_macro));
		if (num_macro > 0)
		{
			macros_ = MakeSharedPtr<std::remove_reference<decltype(*macros_)>::type>();
			macros_->resize(num_macro);
			for (uint32_t i = 0; i < num_macro; ++ i)
			{
				std::string name = ReadShortString(res);
				std::string value = ReadShortString(res);
				(*macros_)[i] = {std::move(name), std::move(value)};
			}
		}

		RasterizerStateDesc rs_desc;
		DepthStencilStateDesc dss_desc;
		BlendStateDesc bs_desc;

		res.read(&rs_desc, sizeof(rs_desc));
		rs_desc.polygon_mode = LE2Native(rs_desc.polygon_mode);
		rs_desc.shade_mode = LE2Native(rs_desc.shade_mode);
		rs_desc.cull_mode = LE2Native(rs_desc.cull_mode);
		rs_desc.polygon_offset_factor = LE2Native(rs_desc.polygon_offset_factor);
		rs_desc.polygon_offset_units = LE2Native(rs_desc.polygon_offset_units);

		res.read(&dss_desc, sizeof(dss_desc));
		dss_desc.depth_func = LE2Native(dss_desc.depth_func);
		dss_desc.front_stencil_func = LE2Native(dss_desc.front_stencil_func);
		dss_desc.front_stencil_ref = LE2Native(dss_desc.front_stencil_ref);
		dss_desc.front_stencil_read_mask = LE2Native(dss_desc.front_stencil_read_mask);
		dss_desc.front_stencil_write_mask = LE2Native(dss_desc.front_stencil_write_mask);
		dss_desc.front_stencil_fail = LE2Native(dss_desc.front_stencil_fail);
		dss_desc.front_stencil_depth_fail = LE2Native(dss_desc.front_stencil_depth_fail);
		dss_desc.front_stencil_pass = LE2Native(dss_desc.front_stencil_pass);
		dss_desc.back_stencil_func = LE2Native(dss_desc.back_stencil_func);
		dss_desc.back_stencil_ref = LE2Native(dss_desc.back_stencil_ref);
		dss_desc.back_stencil_read_mask = LE2Native(dss_desc.back_stencil_read_mask);
		dss_desc.back_stencil_write_mask = LE2Native(dss_desc.back_stencil_write_mask);
		dss_desc.back_stencil_fail = LE2Native(dss_desc.back_stencil_fail);
		dss_desc.back_stencil_depth_fail = LE2Native(dss_desc.back_stencil_depth_fail);
		dss_desc.back_stencil_pass = LE2Native(dss_desc.back_stencil_pass);

		res.read(&bs_desc, sizeof(bs_desc));
		for (size_t i = 0; i < 4; ++ i)
		{
			bs_desc.blend_factor[i] = LE2Native(bs_desc.blend_factor[i]);
		}
		bs_desc.sample_mask = LE2Native(bs_desc.sample_mask);
		for (size_t i = 0; i < bs_desc.blend_op.size(); ++ i)
		{
			bs_desc.blend_op[i] = LE2Native(bs_desc.blend_op[i]);
			bs_desc.src_blend[i] = LE2Native(bs_desc.src_blend[i]);
			bs_desc.dest_blend[i] = LE2Native(bs_desc.dest_blend[i]);
			bs_desc.blend_op_alpha[i] = LE2Native(bs_desc.blend_op_alpha[i]);
			bs_desc.src_blend_alpha[i] = LE2Native(bs_desc.src_blend_alpha[i]);
			bs_desc.dest_blend_alpha[i] = LE2Native(bs_desc.dest_blend_alpha[i]);
		}

		render_state_obj_ = rf.MakeRenderStateObject(rs_desc, dss_desc, bs_desc);

		res.read(&shader_desc_ids_[0], shader_desc_ids_.size() * sizeof(shader_desc_ids_[0]));
		for (uint32_t stage = 0; stage < ShaderStageNum; ++stage)
		{
			shader_desc_ids_[stage] = LE2Native(shader_desc_ids_[stage]);
		}

		shader_obj_index_ = effect.AddShaderObject();
		auto const& shader_obj = this->GetShaderObject(effect);

		bool native_accepted = true;
		for (uint32_t stage_index = 0; stage_index < ShaderStageNum; ++stage_index)
		{
			ShaderDesc const& sd = effect.GetShaderDesc(shader_desc_ids_[stage_index]);
			if (!sd.func_name.empty())
			{
				ShaderStage const stage = static_cast<ShaderStage>(stage_index);

				ShaderStageObjectPtr shader_stage;
				if (sd.tech_pass_type == (tech_index << 16) + (pass_index << 8) + stage_index)
				{
					shader_stage = rf.MakeShaderStageObject(stage);
					if (!shader_stage)
					{
						LogError() << "StreamIn: MakeShaderStageObject returned null for stage "
								   << stage_index << std::endl;
						native_accepted = false;
						continue;
					}
					shader_stage->StreamIn(effect, shader_desc_ids_, res);
				}
				else
				{
					auto const& tech = *effect.TechniqueByIndex(sd.tech_pass_type >> 16);
					auto const& pass = tech.Pass((sd.tech_pass_type >> 8) & 0xFF);
					shader_stage = pass.GetShaderObject(effect)->Stage(stage);
				}

				shader_obj->AttachStage(stage, shader_stage);

				native_accepted &= shader_stage && shader_stage->Validate();
			}
		}

		return native_accepted;
	}

#if ZENGINE_IS_DEV_PLATFORM
	void RenderPass::StreamOut(RenderEffect const & effect, std::ostream& os, uint32_t tech_index, uint32_t pass_index) const
	{
		WriteShortString(os, name_);

		uint8_t num_anno;
		if (annotations_)
		{
			num_anno = static_cast<uint8_t>(annotations_->size());
		}
		else
		{
			num_anno = 0;
		}
		os.write(reinterpret_cast<char const *>(&num_anno), sizeof(num_anno));
		for (uint32_t i = 0; i < num_anno; ++ i)
		{
			RenderEffectAnnotationPtr annotation = MakeSharedPtr<RenderEffectAnnotation>();
			(*annotations_)[i] = annotation;

			annotation->StreamOut(os);
		}

		uint8_t num_macro;
		if (macros_)
		{
			num_macro = static_cast<uint8_t>(macros_->size());
		}
		else
		{
			num_macro = 0;
		}
		os.write(reinterpret_cast<char const *>(&num_macro), sizeof(num_macro));
		for (uint32_t i = 0; i < num_macro; ++ i)
		{
			WriteShortString(os, (*macros_)[i].first);
			WriteShortString(os, (*macros_)[i].second);
		}

		RasterizerStateDesc rs_desc = render_state_obj_->GetRasterizerStateDesc();
		DepthStencilStateDesc dss_desc = render_state_obj_->GetDepthStencilStateDesc();
		BlendStateDesc bs_desc = render_state_obj_->GetBlendStateDesc();

		rs_desc.polygon_mode = Native2LE(rs_desc.polygon_mode);
		rs_desc.shade_mode = Native2LE(rs_desc.shade_mode);
		rs_desc.cull_mode = Native2LE(rs_desc.cull_mode);
		rs_desc.polygon_offset_factor = Native2LE(rs_desc.polygon_offset_factor);
		rs_desc.polygon_offset_units = Native2LE(rs_desc.polygon_offset_units);
		os.write(reinterpret_cast<char const *>(&rs_desc), sizeof(rs_desc));

		dss_desc.depth_func = Native2LE(dss_desc.depth_func);
		dss_desc.front_stencil_func = Native2LE(dss_desc.front_stencil_func);
		dss_desc.front_stencil_ref = Native2LE(dss_desc.front_stencil_ref);
		dss_desc.front_stencil_read_mask = Native2LE(dss_desc.front_stencil_read_mask);
		dss_desc.front_stencil_write_mask = Native2LE(dss_desc.front_stencil_write_mask);
		dss_desc.front_stencil_fail = Native2LE(dss_desc.front_stencil_fail);
		dss_desc.front_stencil_depth_fail = Native2LE(dss_desc.front_stencil_depth_fail);
		dss_desc.front_stencil_pass = Native2LE(dss_desc.front_stencil_pass);
		dss_desc.back_stencil_func = Native2LE(dss_desc.back_stencil_func);
		dss_desc.back_stencil_ref = Native2LE(dss_desc.back_stencil_ref);
		dss_desc.back_stencil_read_mask = Native2LE(dss_desc.back_stencil_read_mask);
		dss_desc.back_stencil_write_mask = Native2LE(dss_desc.back_stencil_write_mask);
		dss_desc.back_stencil_fail = Native2LE(dss_desc.back_stencil_fail);
		dss_desc.back_stencil_depth_fail = Native2LE(dss_desc.back_stencil_depth_fail);
		dss_desc.back_stencil_pass = Native2LE(dss_desc.back_stencil_pass);
		os.write(reinterpret_cast<char const *>(&dss_desc), sizeof(dss_desc));

		for (size_t i = 0; i < 4; ++ i)
		{
			bs_desc.blend_factor[i] = Native2LE(bs_desc.blend_factor[i]);
		}
		bs_desc.sample_mask = Native2LE(bs_desc.sample_mask);
		for (size_t i = 0; i < bs_desc.blend_op.size(); ++ i)
		{
			bs_desc.blend_op[i] = Native2LE(bs_desc.blend_op[i]);
			bs_desc.src_blend[i] = Native2LE(bs_desc.src_blend[i]);
			bs_desc.dest_blend[i] = Native2LE(bs_desc.dest_blend[i]);
			bs_desc.blend_op_alpha[i] = Native2LE(bs_desc.blend_op_alpha[i]);
			bs_desc.src_blend_alpha[i] = Native2LE(bs_desc.src_blend_alpha[i]);
			bs_desc.dest_blend_alpha[i] = Native2LE(bs_desc.dest_blend_alpha[i]);
		}
		os.write(reinterpret_cast<char const *>(&bs_desc), sizeof(bs_desc));

		for (uint32_t i = 0; i < shader_desc_ids_.size(); ++ i)
		{
			uint32_t tmp = Native2LE(shader_desc_ids_[i]);
			os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
		}

		for (uint32_t stage = 0; stage < ShaderStageNum; ++stage)
		{
			ShaderDesc const& sd = effect.GetShaderDesc(shader_desc_ids_[stage]);
			if (!sd.func_name.empty())
			{
				if (sd.tech_pass_type == (tech_index << 16) + (pass_index << 8) + stage)
				{
					this->GetShaderObject(effect)->Stage(static_cast<ShaderStage>(stage))->StreamOut(os);
				}
			}
		}
	}
#endif

	void RenderPass::Bind(RenderEffect const & effect) const
	{
		auto& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		re.SetStateObject(render_state_obj_);

		this->GetShaderObject(effect)->Bind(effect);
	}

	void RenderPass::Unbind(RenderEffect const & effect) const
	{
		this->GetShaderObject(effect)->Unbind();
	}

	RenderEffectAnnotation const& RenderPass::Annotation(uint32_t n) const noexcept
	{
		COMMON_ASSERT(n < this->NumAnnotations());
		return *(*annotations_)[n];
	}

	std::pair<std::string, std::string> const& RenderPass::MacroByIndex(uint32_t n) const noexcept
	{
		COMMON_ASSERT(n < this->NumMacros());
		return (*macros_)[n];
	}


	RenderEffectConstantBuffer::Immutable::Immutable() = default;

	RenderEffectConstantBuffer::RenderEffectConstantBuffer(RenderEffect& effect) : effect_(effect)
	{
	}

#if ZENGINE_IS_DEV_PLATFORM
	void RenderEffectConstantBuffer::Load(std::string const & name)
	{
		if (!immutable_)
		{
			immutable_ = MakeSharedPtr<Immutable>();
		}

		immutable_->name = name;
		immutable_->name_hash = HashValue(name);
		param_indices_ = MakeSharedPtr<std::remove_reference<decltype(*param_indices_)>::type>();
	}
#endif

	void RenderEffectConstantBuffer::StreamIn(ResIdentifier& res)
	{
		if (!immutable_)
		{
			immutable_ = MakeSharedPtr<Immutable>();
		}

		immutable_->name = ReadShortString(res);
		immutable_->name_hash = HashValue(immutable_->name);
		param_indices_ = MakeSharedPtr<std::remove_reference<decltype(*param_indices_)>::type>();

		uint16_t len;
		res.read(&len, sizeof(len));
		len = LE2Native(len);
		param_indices_->resize(len);
		res.read(param_indices_->data(), len * sizeof((*param_indices_)[0]));
		for (auto& param_index : *param_indices_)
		{
			param_index = LE2Native(param_index);
		}
	}

#if ZENGINE_IS_DEV_PLATFORM
	void RenderEffectConstantBuffer::StreamOut(std::ostream& os) const
	{
		WriteShortString(os, immutable_->name);

		uint16_t len = Native2LE(static_cast<uint16_t>(param_indices_->size()));
		os.write(reinterpret_cast<char const *>(&len), sizeof(len));
		for (auto param_index : *param_indices_)
		{
			uint32_t const tmp = Native2LE(param_index);
			os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
		}
	}
#endif

	RenderEffectConstantBufferPtr RenderEffectConstantBuffer::Clone(RenderEffect& dst_effect)
	{
		auto ret = MakeSharedPtr<RenderEffectConstantBuffer>(dst_effect);
		this->Reclone(*ret, dst_effect);
		return ret;
	}

	void RenderEffectConstantBuffer::Reclone(RenderEffectConstantBuffer& dst_cbuffer, RenderEffect& dst_effect)
	{
		if (effect_.ResNameHash() == dst_effect.ResNameHash())
		{
			dst_cbuffer.param_indices_ = param_indices_;
		}
		else
		{
			dst_cbuffer.param_indices_ = MakeSharedPtr<std::vector<uint32_t>>(param_indices_->size());
		}
		dst_cbuffer.immutable_ = immutable_;
		dst_cbuffer.buff_ = buff_;
		dst_cbuffer.Resize(static_cast<uint32_t>(buff_.size()));

		this->RebindParameters(dst_cbuffer, dst_effect);
	}

	void RenderEffectConstantBuffer::RebindParameters(RenderEffectConstantBuffer& dst_cbuffer, RenderEffect& dst_effect)
	{
		if (&effect_ != &dst_effect)
		{
			for (uint32_t i = 0; i < param_indices_->size(); ++i)
			{
				uint32_t param_index = (*param_indices_)[i];
				const RenderEffectParameter* src_param = effect_.ParameterByIndex(param_index);
				if (src_param->InCBuffer())
				{
					if (effect_.ResNameHash() != dst_effect.ResNameHash())
					{
						for (uint32_t j = 0; j < dst_effect.NumParameters(); ++j)
						{
							if (dst_effect.ParameterByIndex(j)->NameHash() == src_param->NameHash())
							{
								param_index = j;
								break;
							}
						}
					}

					RenderEffectParameter* dst_param = dst_effect.ParameterByIndex(param_index);
					if (dst_param->InCBuffer())
					{
						dst_param->RebindToCBuffer(dst_effect, src_param->CBufferIndex());
					}
					else
					{
						dst_param->BindToCBuffer(dst_effect, src_param->CBufferIndex(), src_param->CBufferOffset(), src_param->Stride());
					}

					(*dst_cbuffer.param_indices_)[i] = param_index;
				}
			}
		}
	}

	void RenderEffectConstantBuffer::AddParameter(uint32_t index)
	{
		param_indices_->push_back(index);
	}

	uint32_t RenderEffectConstantBuffer::ParameterIndex(uint32_t index) const noexcept
	{
		COMMON_ASSERT(index < this->NumParameters());
		return (*param_indices_)[index];
	}

	void RenderEffectConstantBuffer::Resize(uint32_t size)
	{
		buff_.resize(size);
		if (size > 0)
		{
			if (!hw_buff_ || (size > hw_buff_->Size()))
			{
				RenderFactory& rf = Context::Instance().RenderFactoryInstance();
				hw_buff_ = rf.MakeConstantBuffer(BU_Dynamic, 0, size, nullptr);
			}
		}

		dirty_ = true;
	}

	void RenderEffectConstantBuffer::Update()
	{
		if (dirty_)
		{
			hw_buff_->UpdateSubresource(0, static_cast<uint32_t>(buff_.size()), &buff_[0]);

			dirty_ = false;
		}
	}

	void RenderEffectConstantBuffer::BindHWBuff(GraphicsBufferPtr const & buff)
	{
		hw_buff_ = buff;
		buff_.resize(buff->Size());
	}

	RenderEffectParameter::Immutable::Immutable() = default;

	RenderEffectParameter::RenderEffectParameter() = default;
	RenderEffectParameter::RenderEffectParameter(RenderEffectParameter&& rhs) noexcept = default;
	RenderEffectParameter& RenderEffectParameter::operator=(RenderEffectParameter&& rhs) noexcept = default;

	void RenderEffectParameter::StreamIn(RenderEffect const& effect, ResIdentifier& res)
	{
		if (!immutable_)
		{
			immutable_ = MakeSharedPtr<Immutable>();
		}

		res.read(&immutable_->type, sizeof(immutable_->type));
		immutable_->type = LE2Native(immutable_->type);
		immutable_->name = ReadShortString(res);
		immutable_->name_hash = HashValue(immutable_->name);

		std::string sem = ReadShortString(res);
		if (!sem.empty())
		{
			immutable_->semantic = sem;
		}
		else
		{
			immutable_->semantic.clear();
		}
		immutable_->semantic_hash = HashValue(sem);

		uint32_t as;
		std::string as_str = ReadShortString(res);
		if (as_str.empty())
		{
			as = 0;
		}
		else
		{
			immutable_->array_size = MakeUniquePtr<std::string>(as_str);

			char const* str = as_str.c_str();
			std::from_chars_result result = std::from_chars(str, str + as_str.size(), as);
			if (result.ec != std::errc())
			{
				as = 1;  // dummy array size
			}
		}
		var_ = StreamInVariable(effect, res, immutable_->type, as);

		uint8_t num_anno;
		res.read(&num_anno, sizeof(num_anno));
		if (num_anno > 0)
		{
			immutable_->annotations = MakeUniquePtr<std::remove_reference<decltype(*immutable_->annotations)>::type>();
			immutable_->annotations->resize(num_anno);
			for (uint32_t i = 0; i < num_anno; ++ i)
			{
				RenderEffectAnnotation& anno = (*immutable_->annotations)[i];
				anno.StreamIn(effect, res);
				this->ProcessAnnotation(anno);
			}
		}
	}

	void RenderEffectParameter::ProcessAnnotation(RenderEffectAnnotation& anno)
	{
		if (((REDT_texture1D == immutable_->type) || (REDT_texture2D == immutable_->type) || (REDT_texture2DMS == immutable_->type) ||
				(REDT_texture3D == immutable_->type) || (REDT_textureCUBE == immutable_->type) ||
				(REDT_texture1DArray == immutable_->type) || (REDT_texture2DArray == immutable_->type) ||
				(REDT_texture2DMSArray == immutable_->type) || (REDT_texture3DArray == immutable_->type) ||
				(REDT_textureCUBEArray == immutable_->type)) &&
			(REDT_string == anno.Type()) && (anno.Name() == "SasResourceAddress"))
		{
			std::string val;
			anno.Value(val);

			if (Context::Instance().ResLoaderInstance().Locate(val).empty())
			{
				LogError() << val << " NOT found" << std::endl;
			}
			else
			{
				*var_ = SyncLoadTexture(val, EAH_GPU_Read | EAH_Immutable);
			}
		}
	}

#if ZENGINE_IS_DEV_PLATFORM
void RenderEffectParameter::StreamOut(std::ostream& os) const
{
	uint32_t t = Native2LE(immutable_->type);
	os.write(reinterpret_cast<char const *>(&t), sizeof(t));
	WriteShortString(os, immutable_->name);
	if (!immutable_->semantic.empty())
	{
		WriteShortString(os, immutable_->semantic);
	}
	else
	{
		uint8_t len = 0;
		os.write(reinterpret_cast<char const *>(&len), sizeof(len));
	}

	if (immutable_->array_size)
	{
		WriteShortString(os, *immutable_->array_size);
	}
	else
	{
		uint8_t len = 0;
		os.write(reinterpret_cast<char const *>(&len), sizeof(len));
	}
	StreamOutVariable(os, *var_);

	uint8_t num_anno;
	if (immutable_->annotations)
	{
		num_anno = static_cast<uint8_t>(immutable_->annotations->size());
	}
	else
	{
		num_anno = 0;
	}
	os.write(reinterpret_cast<char const *>(&num_anno), sizeof(num_anno));
	for (uint32_t i = 0; i < num_anno; ++ i)
	{
		(*immutable_->annotations)[i].StreamOut(os);
	}
}
#endif

RenderEffectParameter RenderEffectParameter::Clone()
{
	RenderEffectParameter ret;

	ret.immutable_ = immutable_;
	ret.var_ = var_->Clone();

	return ret;
}

RenderVariable const& RenderEffectParameter::Var() const noexcept
{
	COMMON_ASSERT(var_);
	return *var_;
}

std::string const & RenderEffectParameter::Semantic() const
{
	if (this->HasSemantic())
	{
		return immutable_->semantic;
	}
	else
	{
		static std::string empty("");
		return empty;
	}
}

size_t RenderEffectParameter::SemanticHash() const noexcept
{
	return this->HasSemantic() ? immutable_->semantic_hash : 0;
}

const RenderEffectAnnotation& RenderEffectParameter::Annotation(uint32_t n) const noexcept
{
	COMMON_ASSERT(n < this->NumAnnotations());
	return (*immutable_->annotations)[n];
}

void RenderEffectParameter::BindToCBuffer(RenderEffect const& effect, uint32_t cbuff_index, uint32_t offset, uint32_t stride)
{
	var_->BindToCBuffer(effect, cbuff_index, offset, stride);
}

void RenderEffectParameter::RebindToCBuffer(RenderEffect const& effect, uint32_t cbuff_index)
{
	var_->RebindToCBuffer(effect, cbuff_index);
}

RenderEffectConstantBuffer& RenderEffectParameter::CBuffer() const
{
	COMMON_ASSERT(this->InCBuffer());
	return *var_->CBuffer();
}

void RenderShaderFragment::StreamIn(ResIdentifier& res)
{
	uint32_t tmp;
	res.read(&tmp, sizeof(tmp));
	stage_ = static_cast<ShaderStage>(LE2Native(tmp));
	res.read(&ver_, sizeof(ver_));

	uint32_t len;
	res.read(&len, sizeof(len));
	len = LE2Native(len);
	str_.resize(len);
	res.read(&str_[0], len * sizeof(str_[0]));
}

#if ZENGINE_IS_DEV_PLATFORM
void RenderShaderFragment::StreamOut(std::ostream& os) const
{
	uint32_t tmp;
	tmp = Native2LE(std::to_underlying(stage_));
	os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
	os.write(reinterpret_cast<char const *>(&ver_), sizeof(ver_));

	uint32_t len = static_cast<uint32_t>(str_.size());
	tmp = Native2LE(len);
	os.write(reinterpret_cast<char const *>(&tmp), sizeof(tmp));
	os.write(&str_[0], len * sizeof(str_[0]));
}
#endif

void RenderShaderGraphNode::StreamIn(ResIdentifier& res)
{
	name_ = ReadShortString(res);
	name_hash_ = HashValue(name_);

	return_type_ = ReadShortString(res);
	impl_ = ReadShortString(res);

	uint8_t len;
	res.read(&len, sizeof(len));
	params_.clear();
	params_.reserve(len);
	for (uint32_t i = 0; i < len; ++ i)
	{
		params_.emplace_back(ReadShortString(res), ReadShortString(res));
	}
}

#if ZENGINE_IS_DEV_PLATFORM
void RenderShaderGraphNode::StreamOut(std::ostream& os) const
{
	WriteShortString(os, name_);
	WriteShortString(os, return_type_);
	WriteShortString(os, impl_);

	uint8_t len = static_cast<uint8_t>(params_.size());
	os.write(reinterpret_cast<char*>(&len), sizeof(len));
	for (uint32_t i = 0; i < len; ++ i)
	{
		WriteShortString(os, params_[i].first);
		WriteShortString(os, params_[i].second);
	}
}
#endif

std::pair<std::string, std::string> const& RenderShaderGraphNode::Parameter(uint32_t n) const noexcept
{
	COMMON_ASSERT(n < this->NumParameters());
	return params_[n];
}

void RenderShaderGraphNode::OverrideImpl(std::string_view impl)
{
	impl_ = std::string(std::move(impl));
}

#if ZENGINE_IS_DEV_PLATFORM
std::string RenderShaderGraphNode::GenDeclarationCode() const
{
	std::string ret;

	ret += return_type_;
	ret += ' ';
	ret += name_;
	ret += '(';
	for (size_t i = 0; i < params_.size(); ++ i)
	{
		auto const & param = params_[i];

		ret += param.first;
		ret += ' ';
		ret += param.second;

		if (i != params_.size() - 1)
		{
			ret += ", ";
		}
	}
	ret += ");\n";

	return ret;
}

std::string RenderShaderGraphNode::GenDefinitionCode() const
{
	std::string ret;

	ret += return_type_;
	ret += ' ';
	ret += name_;
	ret += '(';
	for (size_t i = 0; i < params_.size(); ++ i)
	{
		auto const & param = params_[i];

		ret += param.first;
		ret += ' ';
		ret += param.second;

		if (i != params_.size() - 1)
		{
			ret += ", ";
		}
	}
	ret += ")\n";
	ret += "{\n";
	ret += "\t";
	if (return_type_ != "void")
	{
		ret += "return ";
	}
	ret += impl_;
	ret += '(';
	for (size_t i = 0; i < params_.size(); ++ i)
	{
		auto const & param = params_[i];

		ret += param.second;

		if (i != params_.size() - 1)
		{
			ret += ", ";
		}
	}
	ret += ");\n";
	ret += "}\n\n";

	return ret;
}
#endif

class EffectLoadingDesc : public ResLoadingDesc
{
private:
	struct EffectResourceData
	{
		std::vector<std::string> res_name;

		bool cloned = false;
		RenderEffectPtr effect;
	};

public:
    explicit EffectLoadingDesc(EffectDesc const& desc)
        : definition_(MakeSharedPtr<EffectDesc>(desc)), description_source_(detail::ReadEffectShaderSource(desc))
    {
        auto const platform = Context::Instance().RenderFactoryInstance().RenderEngineInstance().NativeShaderPlatformName();
        effect_desc_.res_name = {"EffectDesc", desc.CacheName(platform),
            std::to_string(detail::EffectSourceFingerprint(description_source_))};
        effect_desc_.effect = MakeSharedPtr<RenderEffect>();
    }

	uint64_t Type() const override
	{
		return CtHash("EffectLoadingDesc");
	}

	bool StateLess() const override
	{
		return false;
	}

	std::shared_ptr<void> CreateResource() override
	{
        effect_desc_.effect->LoadDescription(*definition_, description_source_);
		return effect_desc_.effect;
	}

	void SubThreadStage() override
	{
		std::lock_guard<std::mutex> lock(main_thread_stage_mutex_);

		RenderEffectPtr const& effect = effect_desc_.effect;
		if (effect && effect->HWResourceReady())
		{
			return;
		}

#if ZENGINE_IS_DEV_PLATFORM
		effect->CompileShaders();
#endif

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();
		if (caps.multithread_res_creating_support)
		{
			this->MainThreadStageNoLock();
		}
	}

	void MainThreadStage() override
	{
		std::lock_guard<std::mutex> lock(main_thread_stage_mutex_);
		this->MainThreadStageNoLock();
	}

	bool HasSubThreadStage() const override
	{
		return true;
	}

	bool Match(ResLoadingDesc const & rhs) const override
	{
		if (this->Type() == rhs.Type())
		{
			EffectLoadingDesc const & eld = static_cast<EffectLoadingDesc const &>(rhs);
			return (effect_desc_.res_name == eld.effect_desc_.res_name);
		}
		return false;
	}

	void CopyDataFrom(ResLoadingDesc const & rhs) override
	{
		COMMON_ASSERT(this->Type() == rhs.Type());

		EffectLoadingDesc const & eld = static_cast<EffectLoadingDesc const &>(rhs);
		effect_desc_.res_name = eld.effect_desc_.res_name;
		effect_desc_.effect = eld.effect_desc_.effect->Clone();
		effect_desc_.cloned = true;
	}

	std::shared_ptr<void> CloneResourceFrom(std::shared_ptr<void> const & resource) override
	{
		auto rhs_effect = std::static_pointer_cast<RenderEffect>(resource);
		if (effect_desc_.cloned)
		{
			rhs_effect->Reclone(*effect_desc_.effect);
		}
		else
		{
			rhs_effect->CloneInPlace(*effect_desc_.effect);
		}
		return std::static_pointer_cast<void>(effect_desc_.effect);
	}

	std::shared_ptr<void> Resource() const override
	{
		return effect_desc_.effect;
	}

private:
	void MainThreadStageNoLock()
	{
		RenderEffectPtr const& effect = effect_desc_.effect;
		if (!effect || !effect->HWResourceReady())
		{
			effect->CreateHwShaders();
		}
	}

private:
	EffectResourceData effect_desc_;
    std::shared_ptr<EffectDesc const> definition_;
    std::string description_source_;
	std::mutex main_thread_stage_mutex_;
};

    RenderEffectPtr SyncLoadRenderEffect(EffectDesc const& desc)
    {
        return Context::Instance().ResLoaderInstance().SyncQueryT<RenderEffect>(MakeSharedPtr<EffectLoadingDesc>(desc));
    }

	RenderEffectPtr SyncLoadRenderEffect(std::string_view effect_name)
	{
		return SyncLoadRenderEffect(DescribeBuiltInEffect(effect_name));
	}

		uint32_t EnsureParameterCBufferOffset(RenderEffect& effect, std::string_view param_name,
			std::string_view cb_name, uint32_t fallback_offset, uint32_t stride)
		{
			RenderEffectParameter* param = effect.ParameterByName(param_name);
			if (!param)
			{
				LogError() << "EnsureParameterCBufferOffset: missing parameter '" << param_name << "'" << std::endl;
				return fallback_offset;
			}
			if (param->InCBuffer())
			{
				return param->CBufferOffset();
			}

			uint32_t const cb_index = effect.FindCBuffer(cb_name);
			if (cb_index == static_cast<uint32_t>(-1))
			{
				LogError() << "EnsureParameterCBufferOffset: missing cbuffer '" << cb_name << "'" << std::endl;
				return fallback_offset;
			}

			// Without DXBC reflection (Metal), cbuffers often stay size 0.
			// BindToCBuffer writes the current value into the buffer — grow first.
			if (RenderEffectConstantBuffer* cb = effect.CBufferByIndex(cb_index))
			{
				uint32_t const need = fallback_offset + 64;
				if (cb->Size() < need)
				{
					cb->Resize(need);
				}
			}

			param->BindToCBuffer(effect, cb_index, fallback_offset, stride);
			return fallback_offset;
		}

RenderEffectPtr ASyncLoadRenderEffect(std::string_view effect_name)
{
	return Context::Instance().ResLoaderInstance().ASyncQueryT<RenderEffect>(
		MakeSharedPtr<EffectLoadingDesc>(DescribeBuiltInEffect(effect_name)));
}

} // namespace RenderWorker
