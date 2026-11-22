// RenderVariable.cpp
//////////////////////////////////////////////////////////////////////////////////

#include <base/ZEngine.h>

#include <render/RenderFactory.h>
#include <render/RenderStateObject.h>
#include <render/RenderView.h>
#include <render/ShaderObject.h>
#include <render/Texture.h>

#include <cstring>
#include <iterator>
#include <variant>
#include <stdexcept>
#include <type_traits>

#include <render/RenderEffect.h>
#include <render/EffectDesc.h>
#include "RenderEffectInternal.h"


namespace RenderWorker
{
namespace detail
{
using namespace CommonWorker;

#if ZENGINE_IS_DEV_PLATFORM
	std::span<std::pair<char const *, size_t> const> GetTypeDefines()
	{
#define NAME_AND_HASH(name) std::make_pair(name, CtHash(name))
		static std::pair<char const *, size_t> const types[] =
		{
			NAME_AND_HASH("bool"),
			NAME_AND_HASH("string"),
			NAME_AND_HASH("texture1D"),
			NAME_AND_HASH("texture2D"),
			NAME_AND_HASH("texture2DMS"),
			NAME_AND_HASH("texture3D"),
			NAME_AND_HASH("textureCUBE"),
			NAME_AND_HASH("texture1DArray"),
			NAME_AND_HASH("texture2DArray"),
			NAME_AND_HASH("texture2DMSArray"),
			NAME_AND_HASH("texture3DArray"),
			NAME_AND_HASH("textureCUBEArray"),
			NAME_AND_HASH("sampler"),
			NAME_AND_HASH("shader"),
			NAME_AND_HASH("uint"),
			NAME_AND_HASH("uint2"),
			NAME_AND_HASH("uint3"),
			NAME_AND_HASH("uint4"),
			NAME_AND_HASH("int"),
			NAME_AND_HASH("int2"),
			NAME_AND_HASH("int3"),
			NAME_AND_HASH("int4"),
			NAME_AND_HASH("float"),
			NAME_AND_HASH("float2"),
			NAME_AND_HASH("float2x2"),
			NAME_AND_HASH("float2x3"),
			NAME_AND_HASH("float2x4"),
			NAME_AND_HASH("float3"),
			NAME_AND_HASH("float3x2"),
			NAME_AND_HASH("float3x3"),
			NAME_AND_HASH("float3x4"),
			NAME_AND_HASH("float4"),
			NAME_AND_HASH("float4x2"),
			NAME_AND_HASH("float4x3"),
			NAME_AND_HASH("float4x4"),
			NAME_AND_HASH("buffer"),
			NAME_AND_HASH("structured_buffer"),
			NAME_AND_HASH("byte_address_buffer"),
			NAME_AND_HASH("rw_buffer"),
			NAME_AND_HASH("rw_structured_buffer"),
			NAME_AND_HASH("rw_texture1D"),
			NAME_AND_HASH("rw_texture2D"),
			NAME_AND_HASH("rw_texture3D"),
			NAME_AND_HASH("rw_texture1DArray"),
			NAME_AND_HASH("rw_texture2DArray"),
			NAME_AND_HASH("rw_byte_address_buffer"),
			NAME_AND_HASH("append_structured_buffer"),
			NAME_AND_HASH("consume_structured_buffer"),
			NAME_AND_HASH("rasterizer_ordered_buffer"),
			NAME_AND_HASH("rasterizer_ordered_byte_address_buffer"),
			NAME_AND_HASH("rasterizer_ordered_structured_buffer"),
			NAME_AND_HASH("rasterizer_ordered_texture1D"),
			NAME_AND_HASH("rasterizer_ordered_texture1DArray"),
			NAME_AND_HASH("rasterizer_ordered_texture2D"),
			NAME_AND_HASH("rasterizer_ordered_texture2DArray"),
			NAME_AND_HASH("rasterizer_ordered_texture3D"),
			NAME_AND_HASH("struct"),
		};
#undef NAME_AND_HASH
		static_assert(std::size(types) == REDT_count);

		return MakeSpan(types);
	}

	std::string_view TypeNameFromCode(RenderEffectDataType type)
	{
		auto const types = GetTypeDefines();
		if (type < types.size())
		{
			return types[type].first;
		}

		ZENGINE_UNREACHABLE("Invalid type");
	}

#endif
	class RenderVariableIOable : public RenderVariable
	{
	public:

		virtual void StreamIn(RenderEffect const& effect, ResIdentifier& res) = 0;

#if ZENGINE_IS_DEV_PLATFORM
		virtual void StreamOut(std::ostream& os) const = 0;
#endif

		using RenderVariable::operator=;
		using RenderVariable::Value;
	};

	template <typename T>
	class RenderVariableConcrete : public RenderVariableIOable
	{
	public:
		explicit RenderVariableConcrete(bool in_cbuff = false)
			: in_cbuff_(in_cbuff)
		{
			if (!in_cbuff_)
			{
				data_ = T{};
			}
		}

		~RenderVariableConcrete() override
		{
		}

		std::unique_ptr<RenderVariable> Clone() override
		{
			auto ret = this->MakeInstance(in_cbuff_);
			auto& concrete = checked_cast<RenderVariableConcrete<T>&>(*ret);
			if (in_cbuff_)
			{
				concrete.data_ = data_;
			}
			T val;
			this->Value(val);
			concrete = val;
			return ret;
		}

		RenderVariable& operator=(T const& value) override
		{
			if (in_cbuff_)
			{
				auto const& cbuff_desc = this->RetrieveCBufferDesc();
				auto* cbuff = this->CBuffer();
				T& val_in_cbuff = *(cbuff->template VariableInBuff<T>(cbuff_desc.offset));
				if (val_in_cbuff != value)
				{
					val_in_cbuff = value;
					cbuff->Dirty(true);
				}
			}
			else
			{
				this->RetrieveT() = value;
			}
			return *this;
		}

		using RenderVariableIOable::operator=;

		void Value(T& val) const override
		{
			if (in_cbuff_)
			{
				auto const& cbuff_desc = this->RetrieveCBufferDesc();
				auto* cbuff = this->CBuffer();
				val = *(cbuff->template VariableInBuff<T>(cbuff_desc.offset));
			}
			else
			{
				val = this->RetrieveT();
			}
		}

		using RenderVariableIOable::Value;

		void BindToCBuffer(RenderEffect const& effect, uint32_t cbuff_index, uint32_t offset, uint32_t stride) override
		{
			if (!in_cbuff_)
			{
				T val = this->RetrieveT();
				in_cbuff_ = true;
				CBufferDesc cbuff_desc;
				cbuff_desc.effect = &effect;
				cbuff_desc.cbuff_index = cbuff_index;
				cbuff_desc.offset = offset;
				cbuff_desc.stride = stride;
				data_ = std::move(cbuff_desc);
				this->operator=(val);
			}
		}

		void RebindToCBuffer(RenderEffect const& effect, uint32_t cbuff_index) override
		{
			COMMON_ASSERT(in_cbuff_);
			auto& cbuff_desc = this->RetrieveCBufferDesc();
			cbuff_desc.effect = &effect;
			cbuff_desc.cbuff_index = cbuff_index;
		}

		bool InCBuffer() const noexcept override
		{
			return in_cbuff_;
		}
		RenderEffectConstantBuffer* CBuffer() const override
		{
			auto& cbuff_desc = this->RetrieveCBufferDesc();
			return cbuff_desc.effect->CBufferByIndex(cbuff_desc.cbuff_index);
		}
		uint32_t CBufferIndex() const override
		{
			return this->RetrieveCBufferDesc().cbuff_index;
		}
		uint32_t CBufferOffset() const override
		{
			return this->RetrieveCBufferDesc().offset;
		}
		uint32_t Stride() const override
		{
			return this->RetrieveCBufferDesc().stride;
		}

	protected:
		T& RetrieveT()
		{
			return std::get<T>(data_);
		}
		T const& RetrieveT() const
		{
			return std::get<T>(data_);
		}

		CBufferDesc& RetrieveCBufferDesc()
		{
			return std::get<CBufferDesc>(data_);
		}
		CBufferDesc const& RetrieveCBufferDesc() const
		{
			return std::get<CBufferDesc>(data_);
		}

		virtual std::unique_ptr<RenderVariable> MakeInstance(bool in_cbuff) = 0;

	protected:
		bool in_cbuff_;
		std::variant<CBufferDesc, T> data_;
	};

	class RenderVariableBool final : public RenderVariableConcrete<bool>
	{
	public:
		explicit RenderVariableBool(bool in_cbuff = false)
			: RenderVariableConcrete<bool>(in_cbuff)
		{
		}

		void StreamIn([[maybe_unused]] RenderEffect const& effect, ResIdentifier& res) override
		{
			bool tmp;
			res.read(&tmp, sizeof(tmp));
			*this = tmp;
		}

#if ZENGINE_IS_DEV_PLATFORM
		void StreamOut(std::ostream& os) const override
		{
			bool tmp;
			this->Value(tmp);
			os.write(reinterpret_cast<char const*>(&tmp), sizeof(tmp));
		}
#endif

		using RenderVariableConcrete<bool>::operator=;

	protected:
		std::unique_ptr<RenderVariable> MakeInstance(bool in_cbuff) override
		{
			return MakeUniquePtr<RenderVariableBool>(in_cbuff);
		}
	};

	class RenderVariableUInt final : public RenderVariableConcrete<uint32_t>
	{
	public:
		explicit RenderVariableUInt(bool in_cbuff = false)
			: RenderVariableConcrete<uint32_t>(in_cbuff)
		{
		}

		void StreamIn([[maybe_unused]] RenderEffect const& effect, ResIdentifier& res) override
		{
			uint32_t tmp;
			res.read(&tmp, sizeof(tmp));
			*this = LE2Native(tmp);
		}

#if ZENGINE_IS_DEV_PLATFORM
		void StreamOut(std::ostream& os) const override
		{
			uint32_t tmp;
			this->Value(tmp);
			tmp = Native2LE(tmp);
			os.write(reinterpret_cast<char const*>(&tmp), sizeof(tmp));
		}
#endif

		using RenderVariableConcrete<uint32_t>::operator=;

	protected:
		std::unique_ptr<RenderVariable> MakeInstance(bool in_cbuff) override
		{
			return MakeUniquePtr<RenderVariableUInt>(in_cbuff);
		}
	};

	class RenderVariableInt final : public RenderVariableConcrete<int32_t>
	{
	public:
		explicit RenderVariableInt(bool in_cbuff = false)
			: RenderVariableConcrete<int32_t>(in_cbuff)
		{
		}

		void StreamIn([[maybe_unused]] RenderEffect const& effect, ResIdentifier& res) override
		{
			int32_t tmp;
			res.read(&tmp, sizeof(tmp));
			*this = LE2Native(tmp);
		}

#if ZENGINE_IS_DEV_PLATFORM
		void StreamOut(std::ostream& os) const override
		{
			int32_t tmp;
			this->Value(tmp);
			tmp = Native2LE(tmp);
			os.write(reinterpret_cast<char const*>(&tmp), sizeof(tmp));
		}
#endif

		using RenderVariableConcrete<int32_t>::operator=;

	protected:
		std::unique_ptr<RenderVariable> MakeInstance(bool in_cbuff) override
		{
			return MakeUniquePtr<RenderVariableInt>(in_cbuff);
		}
	};

	class RenderVariableFloat final : public RenderVariableConcrete<float>
	{
	public:
		explicit RenderVariableFloat(bool in_cbuff = false)
			: RenderVariableConcrete<float>(in_cbuff)
		{
		}

		void StreamIn([[maybe_unused]] RenderEffect const& effect, ResIdentifier& res) override
		{
			float tmp;
			res.read(&tmp, sizeof(tmp));
			*this = LE2Native(tmp);
		}

#if ZENGINE_IS_DEV_PLATFORM
		void StreamOut(std::ostream& os) const override
		{
			float tmp;
			this->Value(tmp);
			tmp = Native2LE(tmp);
			os.write(reinterpret_cast<char const*>(&tmp), sizeof(tmp));
		}
#endif

		using RenderVariableConcrete<float>::operator=;

	protected:
		std::unique_ptr<RenderVariable> MakeInstance(bool in_cbuff) override
		{
			return MakeUniquePtr<RenderVariableFloat>(in_cbuff);
		}
	};

	class RenderVariableUInt2 final : public RenderVariableConcrete<uint2>
	{
	public:
		explicit RenderVariableUInt2(bool in_cbuff = false)
			: RenderVariableConcrete<uint2>(in_cbuff)
		{
		}

		void StreamIn([[maybe_unused]] RenderEffect const& effect, ResIdentifier& res) override
		{
			uint2 tmp;
			res.read(&tmp, sizeof(tmp));
			for (size_t i = 0; i < tmp.size(); ++i)
			{
				tmp[i] = LE2Native(tmp[i]);
			}
			*this = tmp;
		}

#if ZENGINE_IS_DEV_PLATFORM
		void StreamOut(std::ostream& os) const override
		{
			uint2 tmp;
			this->Value(tmp);
			for (size_t i = 0; i < tmp.size(); ++i)
			{
				tmp[i] = Native2LE(tmp[i]);
			}
			os.write(reinterpret_cast<char const*>(&tmp), sizeof(tmp));
		}
#endif

		using RenderVariableConcrete<uint2>::operator=;

	protected:
		std::unique_ptr<RenderVariable> MakeInstance(bool in_cbuff) override
		{
			return MakeUniquePtr<RenderVariableUInt2>(in_cbuff);
		}
	};

	class RenderVariableUInt3 final : public RenderVariableConcrete<uint3>
	{
	public:
		explicit RenderVariableUInt3(bool in_cbuff = false)
			: RenderVariableConcrete<uint3>(in_cbuff)
		{
		}

		void StreamIn([[maybe_unused]] RenderEffect const& effect, ResIdentifier& res) override
		{
			uint3 tmp;
			res.read(&tmp, sizeof(tmp));
			for (size_t i = 0; i < tmp.size(); ++i)
			{
				tmp[i] = LE2Native(tmp[i]);
			}
			*this = tmp;
		}

#if ZENGINE_IS_DEV_PLATFORM
		void StreamOut(std::ostream& os) const override
		{
			uint3 tmp;
			this->Value(tmp);
			for (size_t i = 0; i < tmp.size(); ++i)
			{
				tmp[i] = Native2LE(tmp[i]);
			}
			os.write(reinterpret_cast<char const*>(&tmp), sizeof(tmp));
		}
#endif

		using RenderVariableConcrete<uint3>::operator=;

	protected:
		std::unique_ptr<RenderVariable> MakeInstance(bool in_cbuff) override
		{
			return MakeUniquePtr<RenderVariableUInt3>(in_cbuff);
		}
	};

	class RenderVariableUInt4 final : public RenderVariableConcrete<uint4>
	{
	public:
		explicit RenderVariableUInt4(bool in_cbuff = false)
			: RenderVariableConcrete<uint4>(in_cbuff)
		{
		}

		void StreamIn([[maybe_unused]] RenderEffect const& effect, ResIdentifier& res) override
		{
			uint4 tmp;
			res.read(&tmp, sizeof(tmp));
			for (size_t i = 0; i < tmp.size(); ++i)
			{
				tmp[i] = LE2Native(tmp[i]);
			}
			*this = tmp;
		}

#if ZENGINE_IS_DEV_PLATFORM
		void StreamOut(std::ostream& os) const override
		{
			uint4 tmp;
			this->Value(tmp);
			for (size_t i = 0; i < tmp.size(); ++i)
			{
				tmp[i] = Native2LE(tmp[i]);
			}
			os.write(reinterpret_cast<char const*>(&tmp), sizeof(tmp));
		}
#endif

		using RenderVariableConcrete<uint4>::operator=;

	protected:
		std::unique_ptr<RenderVariable> MakeInstance(bool in_cbuff) override
		{
			return MakeUniquePtr<RenderVariableUInt4>(in_cbuff);
		}
	};

	class RenderVariableInt2 final : public RenderVariableConcrete<int2>
	{
	public:
		explicit RenderVariableInt2(bool in_cbuff = false)
			: RenderVariableConcrete<int2>(in_cbuff)
		{
		}

		void StreamIn([[maybe_unused]] RenderEffect const& effect, ResIdentifier& res) override
		{
			int2 tmp;
			res.read(&tmp, sizeof(tmp));
			for (size_t i = 0; i < tmp.size(); ++i)
			{
				tmp[i] = LE2Native(tmp[i]);
			}
			*this = tmp;
		}

#if ZENGINE_IS_DEV_PLATFORM
		void StreamOut(std::ostream& os) const override
		{
			int2 tmp;
			this->Value(tmp);
			for (size_t i = 0; i < tmp.size(); ++i)
			{
				tmp[i] = Native2LE(tmp[i]);
			}
			os.write(reinterpret_cast<char const*>(&tmp), sizeof(tmp));
		}
#endif

		using RenderVariableConcrete<int2>::operator=;

	protected:
		std::unique_ptr<RenderVariable> MakeInstance(bool in_cbuff) override
		{
			return MakeUniquePtr<RenderVariableInt2>(in_cbuff);
		}
	};

	class RenderVariableInt3 final : public RenderVariableConcrete<int3>
	{
	public:
		explicit RenderVariableInt3(bool in_cbuff = false)
			: RenderVariableConcrete<int3>(in_cbuff)
		{
		}

		void StreamIn([[maybe_unused]] RenderEffect const& effect, ResIdentifier& res) override
		{
			int3 tmp;
			res.read(&tmp, sizeof(tmp));
			for (size_t i = 0; i < tmp.size(); ++i)
			{
				tmp[i] = LE2Native(tmp[i]);
			}
			*this = tmp;
		}

#if ZENGINE_IS_DEV_PLATFORM
		void StreamOut(std::ostream& os) const override
		{
			int3 tmp;
			this->Value(tmp);
			for (size_t i = 0; i < tmp.size(); ++i)
			{
				tmp[i] = Native2LE(tmp[i]);
			}
			os.write(reinterpret_cast<char const*>(&tmp), sizeof(tmp));
		}
#endif

		using RenderVariableConcrete<int3>::operator=;

	protected:
		std::unique_ptr<RenderVariable> MakeInstance(bool in_cbuff) override
		{
			return MakeUniquePtr<RenderVariableInt3>(in_cbuff);
		}
	};

	class RenderVariableInt4 final : public RenderVariableConcrete<int4>
	{
	public:
		explicit RenderVariableInt4(bool in_cbuff = false)
			: RenderVariableConcrete<int4>(in_cbuff)
		{
		}

		void StreamIn([[maybe_unused]] RenderEffect const& effect, ResIdentifier& res) override
		{
			int4 tmp;
			res.read(&tmp, sizeof(tmp));
			for (size_t i = 0; i < tmp.size(); ++i)
			{
				tmp[i] = LE2Native(tmp[i]);
			}
			*this = tmp;
		}

#if ZENGINE_IS_DEV_PLATFORM
		void StreamOut(std::ostream& os) const override
		{
			int4 tmp;
			this->Value(tmp);
			for (size_t i = 0; i < tmp.size(); ++i)
			{
				tmp[i] = Native2LE(tmp[i]);
			}
			os.write(reinterpret_cast<char const*>(&tmp), sizeof(tmp));
		}
#endif

		using RenderVariableConcrete<int4>::operator=;

	protected:
		std::unique_ptr<RenderVariable> MakeInstance(bool in_cbuff) override
		{
			return MakeUniquePtr<RenderVariableInt4>(in_cbuff);
		}
	};

	class RenderVariableFloat2 final : public RenderVariableConcrete<float2>
	{
	public:
		explicit RenderVariableFloat2(bool in_cbuff = false)
			: RenderVariableConcrete<float2>(in_cbuff)
		{
		}

		void StreamIn([[maybe_unused]] RenderEffect const& effect, ResIdentifier& res) override
		{
			float2 tmp;
			res.read(&tmp, sizeof(tmp));
			for (size_t i = 0; i < tmp.size(); ++i)
			{
				tmp[i] = LE2Native(tmp[i]);
			}
			*this = tmp;
		}

#if ZENGINE_IS_DEV_PLATFORM
		void StreamOut(std::ostream& os) const override
		{
			float2 tmp;
			this->Value(tmp);
			for (size_t i = 0; i < tmp.size(); ++i)
			{
				tmp[i] = Native2LE(tmp[i]);
			}
			os.write(reinterpret_cast<char const*>(&tmp), sizeof(tmp));
		}
#endif

		using RenderVariableConcrete<float2>::operator=;

	protected:
		std::unique_ptr<RenderVariable> MakeInstance(bool in_cbuff) override
		{
			return MakeUniquePtr<RenderVariableFloat2>(in_cbuff);
		}
	};

	class RenderVariableFloat3 final : public RenderVariableConcrete<float3>
	{
	public:
		explicit RenderVariableFloat3(bool in_cbuff = false)
			: RenderVariableConcrete<float3>(in_cbuff)
		{
		}

		void StreamIn([[maybe_unused]] RenderEffect const& effect, ResIdentifier& res) override
		{
			float3 tmp;
			res.read(&tmp, sizeof(tmp));
			for (size_t i = 0; i < tmp.size(); ++i)
			{
				tmp[i] = LE2Native(tmp[i]);
			}
			*this = tmp;
		}

#if ZENGINE_IS_DEV_PLATFORM
		void StreamOut(std::ostream& os) const override
		{
			float3 tmp;
			this->Value(tmp);
			for (size_t i = 0; i < tmp.size(); ++i)
			{
				tmp[i] = Native2LE(tmp[i]);
			}
			os.write(reinterpret_cast<char const*>(&tmp), sizeof(tmp));
		}
#endif

		using RenderVariableConcrete<float3>::operator=;

	protected:
		std::unique_ptr<RenderVariable> MakeInstance(bool in_cbuff) override
		{
			return MakeUniquePtr<RenderVariableFloat3>(in_cbuff);
		}
	};

	class RenderVariableFloat4 final : public RenderVariableConcrete<float4>
	{
	public:
		explicit RenderVariableFloat4(bool in_cbuff = false)
			: RenderVariableConcrete<float4>(in_cbuff)
		{
		}

		void StreamIn([[maybe_unused]] RenderEffect const& effect, ResIdentifier& res) override
		{
			float4 tmp;
			res.read(&tmp, sizeof(tmp));
			for (size_t i = 0; i < tmp.size(); ++i)
			{
				tmp[i] = LE2Native(tmp[i]);
			}
			*this = tmp;
		}

#if ZENGINE_IS_DEV_PLATFORM
		void StreamOut(std::ostream& os) const override
		{
			float4 tmp;
			this->Value(tmp);
			for (size_t i = 0; i < tmp.size(); ++i)
			{
				tmp[i] = Native2LE(tmp[i]);
			}
			os.write(reinterpret_cast<char const*>(&tmp), sizeof(tmp));
		}
#endif

		using RenderVariableConcrete<float4>::operator=;

	protected:
		std::unique_ptr<RenderVariable> MakeInstance(bool in_cbuff) override
		{
			return MakeUniquePtr<RenderVariableFloat4>(in_cbuff);
		}
	};

	class RenderVariableFloat4x4 final : public RenderVariableConcrete<float4x4>
	{
	public:
		explicit RenderVariableFloat4x4(bool in_cbuff = false)
			: RenderVariableConcrete<float4x4>(in_cbuff)
		{
		}

		std::unique_ptr<RenderVariable> Clone() override
		{
			auto ret = MakeUniquePtr<RenderVariableFloat4x4>(in_cbuff_);
			if (in_cbuff_)
			{
				ret->data_ = data_;

				float4x4 val;
				RenderVariableConcrete<float4x4>::Value(val);
				RenderVariableConcrete<float4x4>::operator=(val);
			}
			else
			{
				ret->RetrieveT() = this->RetrieveT();
			}
			return ret;
		}

		void StreamIn([[maybe_unused]] RenderEffect const& effect, ResIdentifier& res) override
		{
			float4x4 tmp;
			res.read(&tmp, sizeof(tmp));
			for (size_t i = 0; i < tmp.size(); ++i)
			{
				tmp[i] = LE2Native(tmp[i]);
			}
			*this = tmp;
		}

#if ZENGINE_IS_DEV_PLATFORM
		void StreamOut(std::ostream& os) const override
		{
			float4x4 tmp;
			this->Value(tmp);
			for (size_t i = 0; i < tmp.size(); ++i)
			{
				tmp[i] = Native2LE(tmp[i]);
			}
			os.write(reinterpret_cast<char const*>(&tmp), sizeof(tmp));
		}
#endif

		RenderVariable& operator=(float4x4 const& value) override
		{
			return RenderVariableConcrete<float4x4>::operator=(MathWorker::transpose(value));
		}

		using RenderVariableConcrete<float4x4>::operator=;

		void Value(float4x4& val) const override
		{
			RenderVariableConcrete<float4x4>::Value(val);
			val = MathWorker::transpose(val);
		}

		using RenderVariableConcrete<float4x4>::Value;

	protected:
		std::unique_ptr<RenderVariable> MakeInstance(bool in_cbuff) override
		{
			return MakeUniquePtr<RenderVariableFloat4x4>(in_cbuff);
		}
	};

	class RenderVariableSampler final : public RenderVariableConcrete<SamplerStateObjectPtr>
	{
	public:

		void StreamIn([[maybe_unused]] RenderEffect const& effect, ResIdentifier& res) override
		{
			SamplerStateDesc desc;
			res.read(&desc, sizeof(desc));
			desc.border_clr[0] = LE2Native(desc.border_clr[0]);
			desc.border_clr[1] = LE2Native(desc.border_clr[1]);
			desc.border_clr[2] = LE2Native(desc.border_clr[2]);
			desc.border_clr[3] = LE2Native(desc.border_clr[3]);
			desc.addr_mode_u = LE2Native(desc.addr_mode_u);
			desc.addr_mode_v = LE2Native(desc.addr_mode_v);
			desc.addr_mode_w = LE2Native(desc.addr_mode_w);
			desc.filter = LE2Native(desc.filter);
			desc.min_lod = LE2Native(desc.min_lod);
			desc.max_lod = LE2Native(desc.max_lod);
			desc.mip_map_lod_bias = LE2Native(desc.mip_map_lod_bias);
			desc.cmp_func = LE2Native(desc.cmp_func);

			*this = Context::Instance().RenderFactoryInstance().MakeSamplerStateObject(desc);
		}

#if ZENGINE_IS_DEV_PLATFORM
		void StreamOut(std::ostream& os) const override
		{
			SamplerStateObjectPtr tmp;
			this->Value(tmp);
			SamplerStateDesc desc = tmp->GetDesc();
			desc.border_clr[0] = Native2LE(desc.border_clr[0]);
			desc.border_clr[1] = Native2LE(desc.border_clr[1]);
			desc.border_clr[2] = Native2LE(desc.border_clr[2]);
			desc.border_clr[3] = Native2LE(desc.border_clr[3]);
			desc.addr_mode_u = Native2LE(desc.addr_mode_u);
			desc.addr_mode_v = Native2LE(desc.addr_mode_v);
			desc.addr_mode_w = Native2LE(desc.addr_mode_w);
			desc.filter = Native2LE(desc.filter);
			desc.min_lod = Native2LE(desc.min_lod);
			desc.max_lod = Native2LE(desc.max_lod);
			desc.mip_map_lod_bias = Native2LE(desc.mip_map_lod_bias);
			desc.cmp_func = Native2LE(desc.cmp_func);
			os.write(reinterpret_cast<char const*>(&desc), sizeof(desc));
		}
#endif

		using RenderVariableConcrete<SamplerStateObjectPtr>::operator=;

	protected:
		std::unique_ptr<RenderVariable> MakeInstance([[maybe_unused]] bool in_cbuff) override
		{
			return MakeUniquePtr<RenderVariableSampler>();
		}
	};

	class RenderVariableString final : public RenderVariableConcrete<std::string>
	{
	public:

		void StreamIn([[maybe_unused]] RenderEffect const& effect, ResIdentifier& res) override
		{
			*this = ReadShortString(res);
		}

#if ZENGINE_IS_DEV_PLATFORM
		void StreamOut(std::ostream& os) const override
		{
			std::string tmp;
			this->Value(tmp);
			WriteShortString(os, tmp);
		}
#endif

		using RenderVariableConcrete<std::string>::operator=;

	protected:
		std::unique_ptr<RenderVariable> MakeInstance([[maybe_unused]] bool in_cbuff) override
		{
			return MakeUniquePtr<RenderVariableString>();
		}
	};

	class RenderVariableShader final : public RenderVariableConcrete<ShaderDesc>
	{
	public:

		void StreamIn([[maybe_unused]] RenderEffect const& effect, ResIdentifier& res) override
		{
			ShaderDesc desc;
			desc.profile = ReadShortString(res);
			desc.func_name = ReadShortString(res);
			*this = desc;
		}

#if ZENGINE_IS_DEV_PLATFORM
		void StreamOut(std::ostream& os) const override
		{
			ShaderDesc tmp;
			this->Value(tmp);
			WriteShortString(os, tmp.profile);
			WriteShortString(os, tmp.func_name);
		}
#endif

		using RenderVariableConcrete<ShaderDesc>::operator=;

	protected:
		std::unique_ptr<RenderVariable> MakeInstance([[maybe_unused]] bool in_cbuff) override
		{
			return MakeUniquePtr<RenderVariableShader>();
		}
	};

	template <typename T>
	class RenderVariableArray : public RenderVariableConcrete<std::vector<T>>
	{
	public:
		explicit RenderVariableArray(bool in_cbuff = false)
			: RenderVariableConcrete<std::vector<T>>(in_cbuff)
		{
		}

		std::unique_ptr<RenderVariable> Clone() override
		{
			auto ret = this->MakeInstance(this->in_cbuff_);
			auto& concrete = checked_cast<RenderVariableArray<T>&>(*ret);
			if (this->in_cbuff_)
			{
				concrete.data_ = this->data_;
				concrete.size_ = this->size_;

				auto const& src_cbuff_desc = this->RetrieveCBufferDesc();
				uint8_t const* src = this->CBuffer()->template VariableInBuff<uint8_t>(src_cbuff_desc.offset);

				auto const& dst_cbuff_desc = concrete.RetrieveCBufferDesc();
				uint8_t* dst = concrete.CBuffer()->template VariableInBuff<uint8_t>(dst_cbuff_desc.offset);

				for (size_t i = 0; i < size_; ++i)
				{
					*reinterpret_cast<T*>(dst) = *reinterpret_cast<T const*>(src);
					src += src_cbuff_desc.stride;
					dst += dst_cbuff_desc.stride;
				}

				concrete.CBuffer()->Dirty(true);
			}
			else
			{
				concrete.RetrieveT() = this->RetrieveT();
			}
			return ret;
		}

		RenderVariable& operator=(std::vector<T> const& value) override
		{
			return this->operator=(MakeSpan(value));
		}

		RenderVariable& operator=(std::span<T const> value) override
		{
			if (this->in_cbuff_)
			{
				const uint8_t* src = reinterpret_cast<const uint8_t*>(value.data());

				auto const& cbuff_desc = this->RetrieveCBufferDesc();
				uint8_t* dst = this->CBuffer()->template VariableInBuff<uint8_t>(cbuff_desc.offset);

				size_ = static_cast<uint32_t>(value.size());
				for (size_t i = 0; i < value.size(); ++i)
				{
					*reinterpret_cast<T*>(dst) = *reinterpret_cast<const T*>(src);
					src += sizeof(T);
					dst += cbuff_desc.stride;
				}

				this->CBuffer()->Dirty(true);
			}
			else
			{
				this->RetrieveT() = std::vector<T>(value.begin(), value.end());
			}
			return *this;
		}

		using RenderVariableConcrete<std::vector<T>>::operator=;

		void Value(std::vector<T>& val) const override
		{
			if (this->in_cbuff_)
			{
				auto const& cbuff_desc = this->RetrieveCBufferDesc();
				uint8_t const* src = this->CBuffer()->template VariableInBuff<uint8_t>(cbuff_desc.offset);

				val.resize(size_);

				for (size_t i = 0; i < size_; ++i)
				{
					val[i] = *reinterpret_cast<T const*>(src);
					src += cbuff_desc.stride;
				}
			}
			else
			{
				val = this->RetrieveT();
			}
		}

		using RenderVariableConcrete<std::vector<T>>::Value;

	protected:
		uint32_t size_ = 0;
	};

	class RenderVariableBoolArray final : public RenderVariableArray<uint32_t>
	{
	public:
		explicit RenderVariableBoolArray(bool in_cbuff = false)
			: RenderVariableArray<uint32_t>(in_cbuff)
		{
		}

		void StreamIn([[maybe_unused]] RenderEffect const& effect, ResIdentifier& res) override
		{
			uint32_t len;
			res.read(&len, sizeof(len));
			len = LE2Native(len);
			if (len > 0)
			{
				std::vector<uint8_t> init_val(len);
				res.read(&init_val[0], len * sizeof(init_val[0]));

				std::vector<bool> bool_vec(init_val.size());
				for (size_t i = 0; i < init_val.size(); ++i)
				{
					bool_vec[i] = init_val[i] ? true : false;
				}
				*this = bool_vec;
			}
		}

#if ZENGINE_IS_DEV_PLATFORM
		void StreamOut(std::ostream& os) const override
		{
			std::vector<bool> init_val;
			this->Value(init_val);

			uint32_t len = static_cast<uint32_t>(init_val.size());
			len = Native2LE(len);
			os.write(reinterpret_cast<char const*>(&len), sizeof(len));
			if (len > 0)
			{
				std::vector<uint8_t> const uint8_vec(init_val.begin(), init_val.end());
				os.write(reinterpret_cast<char const*>(uint8_vec.data()), len * sizeof(uint8_vec[0]));
			}
		}
#endif

		RenderVariable& operator=(std::vector<bool> const& value) override
		{
			std::vector<uint8_t> const uint8_vec(value.begin(), value.end());
			return this->operator=(MakeSpan(reinterpret_cast<bool const*>(uint8_vec.data()), uint8_vec.size()));
		}

		RenderVariable& operator=(std::span<bool const> value) override
		{
			if (this->in_cbuff_)
			{
				uint8_t const* src = reinterpret_cast<uint8_t const*>(value.data());

				auto const& cbuff_desc = this->RetrieveCBufferDesc();
				uint8_t* dst = this->CBuffer()->template VariableInBuff<uint8_t>(cbuff_desc.offset);

				size_ = static_cast<uint32_t>(value.size());
				for (size_t i = 0; i < value.size(); ++i)
				{
					*reinterpret_cast<uint32_t*>(dst) = *reinterpret_cast<bool const*>(src);
					src += sizeof(bool);
					dst += cbuff_desc.stride;
				}

				this->CBuffer()->Dirty(true);
			}
			else
			{
				this->RetrieveT() = std::vector<uint32_t>(value.begin(), value.end());
			}
			return *this;
		}

		void Value(std::vector<bool>& val) const override
		{
			if (this->in_cbuff_)
			{
				auto const& cbuff_desc = this->RetrieveCBufferDesc();
				uint8_t const* src = this->CBuffer()->template VariableInBuff<uint8_t>(cbuff_desc.offset);

				val.resize(size_);

				for (size_t i = 0; i < size_; ++i)
				{
					val[i] = *reinterpret_cast<uint32_t const*>(src) ? true : false;
					src += cbuff_desc.stride;
				}
			}
			else
			{
				auto const& uint32_vec = this->RetrieveT();
				val.resize(uint32_vec.size());
				for (size_t i = 0; i < uint32_vec.size(); ++i)
				{
					val[i] = uint32_vec[i] ? true : false;
				}
			}
		}

		using RenderVariableArray<uint32_t>::operator=;
		using RenderVariableArray<uint32_t>::Value;

	protected:
		std::unique_ptr<RenderVariable> MakeInstance(bool in_cbuff) override
		{
			return MakeUniquePtr<RenderVariableBoolArray>(in_cbuff);
		}
	};

	class RenderVariableUIntArray final : public RenderVariableArray<uint32_t>
	{
	public:
		explicit RenderVariableUIntArray(bool in_cbuff = false)
			: RenderVariableArray<uint32_t>(in_cbuff)
		{
		}

		void StreamIn([[maybe_unused]] RenderEffect const& effect, ResIdentifier& res) override
		{
			uint32_t len;
			res.read(&len, sizeof(len));
			len = LE2Native(len);
			if (len > 0)
			{
				std::vector<uint32_t> init_val(len);
				res.read(&init_val[0], len * sizeof(init_val[0]));
				for (uint32_t i = 0; i < len; ++i)
				{
					init_val[i] = LE2Native(init_val[i]);
				}
				*this = init_val;
			}
		}

#if ZENGINE_IS_DEV_PLATFORM
		void StreamOut(std::ostream& os) const override
		{
			std::vector<uint32_t> init_val;
			this->Value(init_val);

			uint32_t const len = static_cast<uint32_t>(init_val.size());
			uint32_t const len_le = Native2LE(len);
			os.write(reinterpret_cast<char const*>(&len_le), sizeof(len_le));
			if (len > 0)
			{
				for (uint32_t i = 0; i < len; ++i)
				{
					init_val[i] = Native2LE(init_val[i]);
				}
				os.write(reinterpret_cast<char const*>(init_val.data()), len * sizeof(init_val[0]));
			}
		}
#endif

		using RenderVariableArray<uint32_t>::operator=;

	protected:
		std::unique_ptr<RenderVariable> MakeInstance(bool in_cbuff) override
		{
			return MakeUniquePtr<RenderVariableUIntArray>(in_cbuff);
		}
	};

	class RenderVariableIntArray final : public RenderVariableArray<int32_t>
	{
	public:
		explicit RenderVariableIntArray(bool in_cbuff = false)
			: RenderVariableArray<int32_t>(in_cbuff)
		{
		}

		void StreamIn([[maybe_unused]] RenderEffect const& effect, ResIdentifier& res) override
		{
			uint32_t len;
			res.read(&len, sizeof(len));
			len = LE2Native(len);
			if (len > 0)
			{
				std::vector<int32_t> init_val(len);
				res.read(&init_val[0], len * sizeof(init_val[0]));
				for (uint32_t i = 0; i < len; ++i)
				{
					init_val[i] = LE2Native(init_val[i]);
				}
				*this = init_val;
			}
		}

#if ZENGINE_IS_DEV_PLATFORM
		void StreamOut(std::ostream& os) const override
		{
			std::vector<int32_t> init_val;
			this->Value(init_val);

			uint32_t const len = static_cast<uint32_t>(init_val.size());
			uint32_t const len_le = Native2LE(len);
			os.write(reinterpret_cast<char const*>(&len_le), sizeof(len_le));
			if (len > 0)
			{
				for (uint32_t i = 0; i < len; ++i)
				{
					init_val[i] = Native2LE(init_val[i]);
				}
				os.write(reinterpret_cast<char const*>(init_val.data()), len * sizeof(init_val[0]));
			}
		}
#endif

		using RenderVariableArray<int32_t>::operator=;

	protected:
		std::unique_ptr<RenderVariable> MakeInstance(bool in_cbuff) override
		{
			return MakeUniquePtr<RenderVariableIntArray>(in_cbuff);
		}
	};

	class RenderVariableFloatArray final : public RenderVariableArray<float>
	{
	public:
		explicit RenderVariableFloatArray(bool in_cbuff = false)
			: RenderVariableArray<float>(in_cbuff)
		{
		}

		void StreamIn([[maybe_unused]] RenderEffect const& effect, ResIdentifier& res) override
		{
			uint32_t len;
			res.read(&len, sizeof(len));
			len = LE2Native(len);
			if (len > 0)
			{
				std::vector<float> init_val(len);
				res.read(&init_val[0], len * sizeof(init_val[0]));
				for (uint32_t i = 0; i < len; ++i)
				{
					init_val[i] = LE2Native(init_val[i]);
				}
				*this = init_val;
			}
		}

#if ZENGINE_IS_DEV_PLATFORM
		void StreamOut(std::ostream& os) const override
		{
			std::vector<float> init_val;
			this->Value(init_val);

			uint32_t const len = static_cast<uint32_t>(init_val.size());
			uint32_t const len_le = Native2LE(len);
			os.write(reinterpret_cast<char const*>(&len_le), sizeof(len_le));
			if (len > 0)
			{
				for (uint32_t i = 0; i < len; ++i)
				{
					init_val[i] = Native2LE(init_val[i]);
				}
				os.write(reinterpret_cast<char const*>(init_val.data()), len * sizeof(init_val[0]));
			}
		}
#endif

		using RenderVariableArray<float>::operator=;

	protected:
		std::unique_ptr<RenderVariable> MakeInstance(bool in_cbuff) override
		{
			return MakeUniquePtr<RenderVariableFloatArray>(in_cbuff);
		}
	};

	class RenderVariableUInt2Array final : public RenderVariableArray<uint2>
	{
	public:
		explicit RenderVariableUInt2Array(bool in_cbuff = false)
			: RenderVariableArray<uint2>(in_cbuff)
		{
		}

		void StreamIn([[maybe_unused]] RenderEffect const& effect, ResIdentifier& res) override
		{
			uint32_t len;
			res.read(&len, sizeof(len));
			len = LE2Native(len);
			if (len > 0)
			{
				std::vector<uint2> init_val(len);
				res.read(&init_val[0], len * sizeof(init_val[0]));
				size_t const dim = init_val[0].size();
				for (uint32_t i = 0; i < len; ++i)
				{
					for (size_t j = 0; j < dim; ++j)
					{
						init_val[i][j] = LE2Native(init_val[i][j]);
					}
				}
				*this = init_val;
			}
		}

#if ZENGINE_IS_DEV_PLATFORM
		void StreamOut(std::ostream& os) const override
		{
			std::vector<uint2> init_val;
			this->Value(init_val);

			uint32_t const len = static_cast<uint32_t>(init_val.size());
			uint32_t const len_le = Native2LE(len);
			os.write(reinterpret_cast<char const*>(&len_le), sizeof(len_le));
			if (len > 0)
			{
				size_t const dim = init_val[0].size();
				for (uint32_t i = 0; i < len; ++i)
				{
					for (size_t j = 0; j < dim; ++j)
					{
						init_val[i][j] = Native2LE(init_val[i][j]);
					}
				}
				os.write(reinterpret_cast<char const*>(init_val.data()), len * sizeof(init_val[0]));
			}
		}
#endif

		using RenderVariableArray<uint2>::operator=;

	protected:
		std::unique_ptr<RenderVariable> MakeInstance(bool in_cbuff) override
		{
			return MakeUniquePtr<RenderVariableUInt2Array>(in_cbuff);
		}
	};

	class RenderVariableUInt3Array final : public RenderVariableArray<uint3>
	{
	public:
		explicit RenderVariableUInt3Array(bool in_cbuff = false)
			: RenderVariableArray<uint3>(in_cbuff)
		{
		}

		void StreamIn([[maybe_unused]] RenderEffect const& effect, ResIdentifier& res) override
		{
			uint32_t len;
			res.read(&len, sizeof(len));
			len = LE2Native(len);
			if (len > 0)
			{
				std::vector<uint3> init_val(len);
				res.read(&init_val[0], len * sizeof(init_val[0]));
				size_t const dim = init_val[0].size();
				for (uint32_t i = 0; i < len; ++i)
				{
					for (size_t j = 0; j < dim; ++j)
					{
						init_val[i][j] = LE2Native(init_val[i][j]);
					}
				}
				*this = init_val;
			}
		}

#if ZENGINE_IS_DEV_PLATFORM
		void StreamOut(std::ostream& os) const override
		{
			std::vector<uint3> init_val;
			this->Value(init_val);

			uint32_t const len = static_cast<uint32_t>(init_val.size());
			uint32_t const len_le = Native2LE(len);
			os.write(reinterpret_cast<char const*>(&len_le), sizeof(len_le));
			if (len > 0)
			{
				size_t const dim = init_val[0].size();
				for (uint32_t i = 0; i < len; ++i)
				{
					for (size_t j = 0; j < dim; ++j)
					{
						init_val[i][j] = Native2LE(init_val[i][j]);
					}
				}
				os.write(reinterpret_cast<char const*>(init_val.data()), len * sizeof(init_val[0]));
			}
		}
#endif

		using RenderVariableArray<uint3>::operator=;

	protected:
		std::unique_ptr<RenderVariable> MakeInstance(bool in_cbuff) override
		{
			return MakeUniquePtr<RenderVariableUInt3Array>(in_cbuff);
		}
	};

	class RenderVariableUInt4Array final : public RenderVariableArray<uint4>
	{
	public:
		explicit RenderVariableUInt4Array(bool in_cbuff = false)
			: RenderVariableArray<uint4>(in_cbuff)
		{
		}

		void StreamIn([[maybe_unused]] RenderEffect const& effect, ResIdentifier& res) override
		{
			uint32_t len;
			res.read(&len, sizeof(len));
			len = LE2Native(len);
			if (len > 0)
			{
				std::vector<uint4> init_val(len);
				res.read(&init_val[0], len * sizeof(init_val[0]));
				size_t const dim = init_val[0].size();
				for (uint32_t i = 0; i < len; ++i)
				{
					for (size_t j = 0; j < dim; ++j)
					{
						init_val[i][j] = LE2Native(init_val[i][j]);
					}
				}
				*this = init_val;
			}
		}

#if ZENGINE_IS_DEV_PLATFORM
		void StreamOut(std::ostream& os) const override
		{
			std::vector<uint4> init_val;
			this->Value(init_val);

			uint32_t const len = static_cast<uint32_t>(init_val.size());
			uint32_t const len_le = Native2LE(len);
			os.write(reinterpret_cast<char const*>(&len_le), sizeof(len_le));
			if (len > 0)
			{
				size_t const dim = init_val[0].size();
				for (uint32_t i = 0; i < len; ++i)
				{
					for (size_t j = 0; j < dim; ++j)
					{
						init_val[i][j] = Native2LE(init_val[i][j]);
					}
				}
				os.write(reinterpret_cast<char const*>(init_val.data()), len * sizeof(init_val[0]));
			}
		}
#endif

		using RenderVariableArray<uint4>::operator=;

	protected:
		std::unique_ptr<RenderVariable> MakeInstance(bool in_cbuff) override
		{
			return MakeUniquePtr<RenderVariableUInt4Array>(in_cbuff);
		}
	};

	class RenderVariableInt2Array final : public RenderVariableArray<int2>
	{
	public:
		explicit RenderVariableInt2Array(bool in_cbuff = false)
			: RenderVariableArray<int2>(in_cbuff)
		{
		}

		void StreamIn([[maybe_unused]] RenderEffect const& effect, ResIdentifier& res) override
		{
			uint32_t len;
			res.read(&len, sizeof(len));
			len = LE2Native(len);
			if (len > 0)
			{
				std::vector<int2> init_val(len);
				res.read(&init_val[0], len * sizeof(init_val[0]));
				size_t const dim = init_val[0].size();
				for (uint32_t i = 0; i < len; ++i)
				{
					for (size_t j = 0; j < dim; ++j)
					{
						init_val[i][j] = LE2Native(init_val[i][j]);
					}
				}
				*this = init_val;
			}
		}

#if ZENGINE_IS_DEV_PLATFORM
		void StreamOut(std::ostream& os) const override
		{
			std::vector<int2> init_val;
			this->Value(init_val);

			uint32_t const len = static_cast<uint32_t>(init_val.size());
			uint32_t const len_le = Native2LE(len);
			os.write(reinterpret_cast<char const*>(&len_le), sizeof(len_le));
			if (len > 0)
			{
				size_t const dim = init_val[0].size();
				for (uint32_t i = 0; i < len; ++i)
				{
					for (size_t j = 0; j < dim; ++j)
					{
						init_val[i][j] = Native2LE(init_val[i][j]);
					}
				}
				os.write(reinterpret_cast<char const*>(init_val.data()), len * sizeof(init_val[0]));
			}
		}
#endif

		using RenderVariableArray<int2>::operator=;

	protected:
		std::unique_ptr<RenderVariable> MakeInstance(bool in_cbuff) override
		{
			return MakeUniquePtr<RenderVariableInt2Array>(in_cbuff);
		}
	};

	class RenderVariableInt3Array final : public RenderVariableArray<int3>
	{
	public:
		explicit RenderVariableInt3Array(bool in_cbuff = false)
			: RenderVariableArray<int3>(in_cbuff)
		{
		}

		void StreamIn([[maybe_unused]] RenderEffect const& effect, ResIdentifier& res) override
		{
			uint32_t len;
			res.read(&len, sizeof(len));
			len = LE2Native(len);
			if (len > 0)
			{
				std::vector<int3> init_val(len);
				res.read(&init_val[0], len * sizeof(init_val[0]));
				size_t const dim = init_val[0].size();
				for (uint32_t i = 0; i < len; ++i)
				{
					for (size_t j = 0; j < dim; ++j)
					{
						init_val[i][j] = LE2Native(init_val[i][j]);
					}
				}
				*this = init_val;
			}
		}

#if ZENGINE_IS_DEV_PLATFORM
		void StreamOut(std::ostream& os) const override
		{
			std::vector<int3> init_val;
			this->Value(init_val);

			uint32_t const len = static_cast<uint32_t>(init_val.size());
			uint32_t const len_le = Native2LE(len);
			os.write(reinterpret_cast<char const*>(&len_le), sizeof(len_le));
			if (len > 0)
			{
				size_t const dim = init_val[0].size();
				for (uint32_t i = 0; i < len; ++i)
				{
					for (size_t j = 0; j < dim; ++j)
					{
						init_val[i][j] = Native2LE(init_val[i][j]);
					}
				}
				os.write(reinterpret_cast<char const*>(init_val.data()), len * sizeof(init_val[0]));
			}
		}
#endif

		using RenderVariableArray<int3>::operator=;

	protected:
		std::unique_ptr<RenderVariable> MakeInstance(bool in_cbuff) override
		{
			return MakeUniquePtr<RenderVariableInt3Array>(in_cbuff);
		}
	};

	class RenderVariableInt4Array final : public RenderVariableArray<int4>
	{
	public:
		explicit RenderVariableInt4Array(bool in_cbuff = false)
			: RenderVariableArray<int4>(in_cbuff)
		{
		}

		void StreamIn([[maybe_unused]] RenderEffect const& effect, ResIdentifier& res) override
		{
			uint32_t len;
			res.read(&len, sizeof(len));
			len = LE2Native(len);
			if (len > 0)
			{
				std::vector<int4> init_val(len);
				res.read(&init_val[0], len * sizeof(init_val[0]));
				size_t const dim = init_val[0].size();
				for (uint32_t i = 0; i < len; ++i)
				{
					for (size_t j = 0; j < dim; ++j)
					{
						init_val[i][j] = LE2Native(init_val[i][j]);
					}
				}
				*this = init_val;
			}
		}

#if ZENGINE_IS_DEV_PLATFORM
		void StreamOut(std::ostream& os) const override
		{
			std::vector<int4> init_val;
			this->Value(init_val);

			uint32_t const len = static_cast<uint32_t>(init_val.size());
			uint32_t const len_le = Native2LE(len);
			os.write(reinterpret_cast<char const*>(&len_le), sizeof(len_le));
			if (len > 0)
			{
				size_t const dim = init_val[0].size();
				for (uint32_t i = 0; i < len; ++i)
				{
					for (size_t j = 0; j < dim; ++j)
					{
						init_val[i][j] = Native2LE(init_val[i][j]);
					}
				}
				os.write(reinterpret_cast<char const*>(init_val.data()), len * sizeof(init_val[0]));
			}
		}
#endif

		using RenderVariableArray<int4>::operator=;

	protected:
		std::unique_ptr<RenderVariable> MakeInstance(bool in_cbuff) override
		{
			return MakeUniquePtr<RenderVariableInt4Array>(in_cbuff);
		}
	};

	class RenderVariableFloat2Array final : public RenderVariableArray<float2>
	{
	public:
		explicit RenderVariableFloat2Array(bool in_cbuff = false)
			: RenderVariableArray<float2>(in_cbuff)
		{
		}

		void StreamIn([[maybe_unused]] RenderEffect const& effect, ResIdentifier& res) override
		{
			uint32_t len;
			res.read(&len, sizeof(len));
			len = LE2Native(len);
			if (len > 0)
			{
				std::vector<float2> init_val(len);
				res.read(&init_val[0], len * sizeof(init_val[0]));
				size_t const dim = init_val[0].size();
				for (uint32_t i = 0; i < len; ++i)
				{
					for (size_t j = 0; j < dim; ++j)
					{
						init_val[i][j] = LE2Native(init_val[i][j]);
					}
				}
				*this = init_val;
			}
		}

#if ZENGINE_IS_DEV_PLATFORM
		void StreamOut(std::ostream& os) const override
		{
			std::vector<float2> init_val;
			this->Value(init_val);

			uint32_t const len = static_cast<uint32_t>(init_val.size());
			uint32_t const len_le = Native2LE(len);
			os.write(reinterpret_cast<char const*>(&len_le), sizeof(len_le));
			if (len > 0)
			{
				size_t const dim = init_val[0].size();
				for (uint32_t i = 0; i < len; ++i)
				{
					for (size_t j = 0; j < dim; ++j)
					{
						init_val[i][j] = Native2LE(init_val[i][j]);
					}
				}
				os.write(reinterpret_cast<char const*>(init_val.data()), len * sizeof(init_val[0]));
			}
		}
#endif

		using RenderVariableArray<float2>::operator=;

	protected:
		std::unique_ptr<RenderVariable> MakeInstance(bool in_cbuff) override
		{
			return MakeUniquePtr<RenderVariableFloat2Array>(in_cbuff);
		}
	};

	class RenderVariableFloat3Array final : public RenderVariableArray<float3>
	{
	public:
		explicit RenderVariableFloat3Array(bool in_cbuff = false)
			: RenderVariableArray<float3>(in_cbuff)
		{
		}

		void StreamIn([[maybe_unused]] RenderEffect const& effect, ResIdentifier& res) override
		{
			uint32_t len;
			res.read(&len, sizeof(len));
			len = LE2Native(len);
			if (len > 0)
			{
				std::vector<float3> init_val(len);
				res.read(&init_val[0], len * sizeof(init_val[0]));
				size_t const dim = init_val[0].size();
				for (uint32_t i = 0; i < len; ++i)
				{
					for (size_t j = 0; j < dim; ++j)
					{
						init_val[i][j] = LE2Native(init_val[i][j]);
					}
				}
				*this = init_val;
			}
		}

#if ZENGINE_IS_DEV_PLATFORM
		void StreamOut(std::ostream& os) const override
		{
			std::vector<float3> init_val;
			this->Value(init_val);

			uint32_t const len = static_cast<uint32_t>(init_val.size());
			uint32_t const len_le = Native2LE(len);
			os.write(reinterpret_cast<char const*>(&len_le), sizeof(len_le));
			if (len > 0)
			{
				size_t const dim = init_val[0].size();
				for (uint32_t i = 0; i < len; ++i)
				{
					for (size_t j = 0; j < dim; ++j)
					{
						init_val[i][j] = Native2LE(init_val[i][j]);
					}
				}
				os.write(reinterpret_cast<char const*>(init_val.data()), len * sizeof(init_val[0]));
			}
		}
#endif

		using RenderVariableArray<float3>::operator=;

	protected:
		std::unique_ptr<RenderVariable> MakeInstance(bool in_cbuff) override
		{
			return MakeUniquePtr<RenderVariableFloat3Array>(in_cbuff);
		}
	};

	class RenderVariableFloat4Array final : public RenderVariableArray<float4>
	{
	public:
		explicit RenderVariableFloat4Array(bool in_cbuff = false)
			: RenderVariableArray<float4>(in_cbuff)
		{
		}

		void StreamIn([[maybe_unused]] RenderEffect const& effect, ResIdentifier& res) override
		{
			uint32_t len;
			res.read(&len, sizeof(len));
			len = LE2Native(len);
			if (len > 0)
			{
				std::vector<float4> init_val(len);
				res.read(&init_val[0], len * sizeof(init_val[0]));
				size_t const dim = init_val[0].size();
				for (uint32_t i = 0; i < len; ++i)
				{
					for (size_t j = 0; j < dim; ++j)
					{
						init_val[i][j] = LE2Native(init_val[i][j]);
					}
				}
				*this = init_val;
			}
		}

#if ZENGINE_IS_DEV_PLATFORM
		void StreamOut(std::ostream& os) const override
		{
			std::vector<float4> init_val;
			this->Value(init_val);

			uint32_t const len = static_cast<uint32_t>(init_val.size());
			uint32_t const len_le = Native2LE(len);
			os.write(reinterpret_cast<char const*>(&len_le), sizeof(len_le));
			if (len > 0)
			{
				size_t const dim = init_val[0].size();
				for (uint32_t i = 0; i < len; ++i)
				{
					for (size_t j = 0; j < dim; ++j)
					{
						init_val[i][j] = Native2LE(init_val[i][j]);
					}
				}
				os.write(reinterpret_cast<char const*>(init_val.data()), len * sizeof(init_val[0]));
			}
		}
#endif

		using RenderVariableArray<float4>::operator=;

	protected:
		std::unique_ptr<RenderVariable> MakeInstance(bool in_cbuff) override
		{
			return MakeUniquePtr<RenderVariableFloat4Array>(in_cbuff);
		}
	};

	class RenderVariableFloat4x4Array final : public RenderVariableConcrete<std::vector<float4x4>>
	{
	public:
		explicit RenderVariableFloat4x4Array(bool in_cbuff = false)
			: RenderVariableConcrete<std::vector<float4x4>>(in_cbuff)
		{
		}

		std::unique_ptr<RenderVariable> Clone() override
		{
			auto ret = MakeUniquePtr<RenderVariableFloat4x4Array>(in_cbuff_);
			if (in_cbuff_)
			{
				ret->data_ = data_;
				ret->size_ = size_;

				auto const& src_cbuff_desc = this->RetrieveCBufferDesc();
				uint8_t const* src = this->CBuffer()->template VariableInBuff<uint8_t>(src_cbuff_desc.offset);

				auto const& dst_cbuff_desc = ret->RetrieveCBufferDesc();
				uint8_t* dst = ret->CBuffer()->template VariableInBuff<uint8_t>(dst_cbuff_desc.offset);

				memcpy(dst, src, size_ * sizeof(float4x4));

				ret->CBuffer()->Dirty(true);
			}
			else
			{
				ret->RetrieveT() = this->RetrieveT();
			}
			return ret;
		}

		void StreamIn([[maybe_unused]] RenderEffect const& effect, ResIdentifier& res) override
		{
			uint32_t len;
			res.read(&len, sizeof(len));
			len = LE2Native(len);
			if (len > 0)
			{
				std::vector<float4x4> init_val(len);
				res.read(&init_val[0], len * sizeof(init_val[0]));
				size_t const dim = init_val[0].size();
				for (uint32_t i = 0; i < len; ++i)
				{
					for (size_t j = 0; j < dim; ++j)
					{
						init_val[i][j] = LE2Native(init_val[i][j]);
					}
				}
				*this = init_val;
			}
		}

#if ZENGINE_IS_DEV_PLATFORM
		void StreamOut(std::ostream& os) const override
		{
			std::vector<float4x4> init_val;
			this->Value(init_val);

			uint32_t const len = static_cast<uint32_t>(init_val.size());
			uint32_t const len_le = Native2LE(len);
			os.write(reinterpret_cast<char const*>(&len_le), sizeof(len_le));
			if (len > 0)
			{
				size_t const dim = init_val[0].size();
				for (uint32_t i = 0; i < len; ++i)
				{
					for (size_t j = 0; j < dim; ++j)
					{
						init_val[i][j] = Native2LE(init_val[i][j]);
					}
				}
				os.write(reinterpret_cast<char const*>(init_val.data()), len * sizeof(init_val[0]));
			}
		}
#endif

		RenderVariable& operator=(std::vector<float4x4> const& value) override
		{
			return this->operator=(MakeSpan(value));
		}

		RenderVariable& operator=(std::span<float4x4 const> value) override
		{
			if (in_cbuff_)
			{
				float4x4 const* src = value.data();

				auto const& cbuff_desc = this->RetrieveCBufferDesc();
				float4x4* dst = this->CBuffer()->template VariableInBuff<float4x4>(cbuff_desc.offset);

				size_ = static_cast<uint32_t>(value.size());
				for (size_t i = 0; i < value.size(); ++i)
				{
					*dst = MathWorker::transpose(*src);
					++src;
					++dst;
				}

				this->CBuffer()->Dirty(true);
			}
			else
			{
				this->RetrieveT() = std::vector<float4x4>(value.begin(), value.end());
			}
			return *this;
		}

		using RenderVariableConcrete<std::vector<float4x4>>::operator=;

		void Value(std::vector<float4x4>& val) const override
		{
			if (in_cbuff_)
			{
				auto const& cbuff_desc = this->RetrieveCBufferDesc();
				float4x4 const* src = this->CBuffer()->template VariableInBuff<float4x4>(cbuff_desc.offset);

				val.resize(size_);
				float4x4* dst = val.data();

				for (size_t i = 0; i < size_; ++i)
				{
					*dst = MathWorker::transpose(*src);
					++src;
					++dst;
				}
			}
			else
			{
				val = this->RetrieveT();
			}
		}

		using RenderVariableConcrete<std::vector<float4x4>>::Value;

	protected:
		std::unique_ptr<RenderVariable> MakeInstance(bool in_cbuff) override
		{
			return MakeUniquePtr<RenderVariableFloat4x4Array>(in_cbuff);
		}

	private:
		uint32_t size_ = 0;
	};

	class RenderVariableTexture final : public RenderVariableIOable
	{
	public:
		std::unique_ptr<RenderVariable> Clone() override
		{
			auto ret = MakeUniquePtr<RenderVariableTexture>();
			TexturePtr val;
			this->Value(val);
			*ret = val;
			std::string elem_type;
			this->Value(elem_type);
			*ret = elem_type;
			return ret;
		}

		void StreamIn([[maybe_unused]] RenderEffect const& effect, ResIdentifier& res) override
		{
			*this = TexturePtr();
			*this = ReadShortString(res);
		}

#if ZENGINE_IS_DEV_PLATFORM
		void StreamOut(std::ostream& os) const override
		{
			std::string tmp;
			this->Value(tmp);
			WriteShortString(os, tmp);
		}
#endif

		RenderVariable& operator=(TexturePtr const& value) override
		{
			if (value)
			{
				uint32_t array_size = value->ArraySize();
				uint32_t mipmap = value->MipMapsNum();

				auto& rf = Context::Instance().RenderFactoryInstance();
				val_ = rf.MakeTextureSrv(value, 0, array_size, 0, mipmap);
			}
			else
			{
				val_.reset();
			}
			return *this;
		}

		RenderVariable& operator=(ShaderResourceViewPtr const& value) override
		{
			val_ = value;
			return *this;
		}

		RenderVariable& operator=(std::string const& value) override
		{
			elem_type_ = value;
			return *this;
		}

		RenderVariable& operator=(std::string_view value) override
		{
			elem_type_ = std::string(std::move(value));
			return *this;
		}

		using RenderVariableIOable::operator=;

		void Value(TexturePtr& val) const override
		{
			if (val_)
			{
				val = val_->TextureResource();
			}
			else
			{
				val.reset();
			}
		}

		void Value(ShaderResourceViewPtr& val) const override
		{
			val = val_;
		}

		void Value(std::string& val) const override
		{
			val = elem_type_;
		}

		void Value(std::string_view& val) const override
		{
			val = elem_type_;
		}

		using RenderVariableIOable::Value;

	protected:
		ShaderResourceViewPtr val_;
		std::string elem_type_;
	};

	class RenderVariableRwTexture final : public RenderVariableIOable
	{
	public:
		std::unique_ptr<RenderVariable> Clone() override
		{
			auto ret = MakeUniquePtr<RenderVariableRwTexture>();
			UnorderedAccessViewPtr val;
			this->Value(val);
			*ret = val;
			std::string elem_type;
			this->Value(elem_type);
			*ret = elem_type;
			return ret;
		}

		void StreamIn([[maybe_unused]] RenderEffect const& effect, ResIdentifier& res) override
		{
			*this = UnorderedAccessViewPtr();
			*this = ReadShortString(res);
		}

#if ZENGINE_IS_DEV_PLATFORM
		void StreamOut(std::ostream& os) const override
		{
			std::string tmp;
			this->Value(tmp);
			WriteShortString(os, tmp);
		}
#endif

		RenderVariable& operator=(TexturePtr const& value) override
		{
			auto& rf = Context::Instance().RenderFactoryInstance();
			switch (value->Type())
			{
			case Texture::TT_1D:
				val_ = rf.Make1DUav(value, 0, static_cast<int>(value->ArraySize()), 0);
				break;

			case Texture::TT_2D:
				val_ = rf.Make2DUav(value, 0, static_cast<int>(value->ArraySize()), 0);
				break;

			case Texture::TT_3D:
				val_ = rf.Make3DUav(value, 0, 0, value->Depth(0), 0);
				break;

			case Texture::TT_Cube:
				val_ = rf.MakeCubeUav(value, 0, 0);
				break;
			}
			return *this;
		}

		RenderVariable& operator=(UnorderedAccessViewPtr const& value) override
		{
			val_ = value;
			return *this;
		}

		RenderVariable& operator=(std::string const& value) override
		{
			elem_type_ = value;
			return *this;
		}

		RenderVariable& operator=(std::string_view value) override
		{
			elem_type_ = std::string(std::move(value));
			return *this;
		}

		using RenderVariableIOable::operator=;

		void Value(TexturePtr& val) const override
		{
			val = val_->TextureResource();
		}

		void Value(UnorderedAccessViewPtr& val) const override
		{
			val = val_;
		}

		void Value(std::string& val) const override
		{
			val = elem_type_;
		}

		void Value(std::string_view& val) const override
		{
			val = elem_type_;
		}

		using RenderVariableIOable::Value;

	protected:
		UnorderedAccessViewPtr val_;
		std::string elem_type_;
	};

	class RenderVariableBuffer final : public RenderVariableIOable
	{
	public:
		std::unique_ptr<RenderVariable> Clone() override
		{
			auto ret = MakeUniquePtr<RenderVariableBuffer>();
			ShaderResourceViewPtr val;
			this->Value(val);
			*ret = val;
			std::string elem_type;
			this->Value(elem_type);
			*ret = elem_type;
			return ret;
		}

		void StreamIn([[maybe_unused]] RenderEffect const& effect, ResIdentifier& res) override
		{
			*this = ShaderResourceViewPtr();
			*this = ReadShortString(res);
		}

#if ZENGINE_IS_DEV_PLATFORM
		void StreamOut(std::ostream& os) const override
		{
			std::string tmp;
			this->Value(tmp);
			WriteShortString(os, tmp);
		}
#endif

		RenderVariable& operator=(ShaderResourceViewPtr const& value) override
		{
			val_ = value;
			return *this;
		}

		RenderVariable& operator=(std::string const& value) override
		{
			elem_type_ = value;
			return *this;
		}

		RenderVariable& operator=(std::string_view value) override
		{
			elem_type_ = std::string(std::move(value));
			return *this;
		}

		using RenderVariableIOable::operator=;

		void Value(ShaderResourceViewPtr& val) const override
		{
			val = val_;
		}

		void Value(std::string& val) const override
		{
			val = elem_type_;
		}

		void Value(std::string_view& val) const override
		{
			val = elem_type_;
		}

		using RenderVariableIOable::Value;

	protected:
		ShaderResourceViewPtr val_;
		std::string elem_type_;
	};

	class RenderVariableRwBuffer final : public RenderVariableIOable
	{
	public:
		std::unique_ptr<RenderVariable> Clone() override
		{
			auto ret = MakeUniquePtr<RenderVariableRwBuffer>();
			UnorderedAccessViewPtr val;
			this->Value(val);
			*ret = val;
			std::string elem_type;
			this->Value(elem_type);
			*ret = elem_type;
			return ret;
		}

		void StreamIn([[maybe_unused]] RenderEffect const& effect, ResIdentifier& res) override
		{
			*this = UnorderedAccessViewPtr();
			*this = ReadShortString(res);
		}

#if ZENGINE_IS_DEV_PLATFORM
		void StreamOut(std::ostream& os) const override
		{
			std::string tmp;
			this->Value(tmp);
			WriteShortString(os, tmp);
		}
#endif

		RenderVariable& operator=(UnorderedAccessViewPtr const& value) override
		{
			val_ = value;
			return *this;
		}

		RenderVariable& operator=(std::string const& value) override
		{
			elem_type_ = value;
			return *this;
		}

		RenderVariable& operator=(std::string_view value) override
		{
			elem_type_ = std::string(std::move(value));
			return *this;
		}

		using RenderVariableIOable::operator=;

		void Value(UnorderedAccessViewPtr& val) const override
		{
			val = val_;
		}

		void Value(std::string& val) const override
		{
			val = elem_type_;
		}

		void Value(std::string_view& val) const override
		{
			val = elem_type_;
		}

		using RenderVariableIOable::Value;

	protected:
		UnorderedAccessViewPtr val_;
		std::string elem_type_;
	};

	class RenderVariableByteAddressBuffer final : public RenderVariableIOable
	{
	public:
		std::unique_ptr<RenderVariable> Clone() override
		{
			auto ret = MakeUniquePtr<RenderVariableByteAddressBuffer>();
			ShaderResourceViewPtr val;
			this->Value(val);
			*ret = val;
			return ret;
		}

		void StreamIn([[maybe_unused]] RenderEffect const& effect, [[maybe_unused]] ResIdentifier& res) override
		{
			*this = ShaderResourceViewPtr();
		}

#if ZENGINE_IS_DEV_PLATFORM
		void StreamOut([[maybe_unused]] std::ostream& os) const override
		{
		}
#endif

		RenderVariable& operator=(ShaderResourceViewPtr const& value) override
		{
			val_ = value;
			return *this;
		}

		using RenderVariableIOable::operator=;

		void Value(ShaderResourceViewPtr& val) const override
		{
			val = val_;
		}

		using RenderVariableIOable::Value;

	protected:
		ShaderResourceViewPtr val_;
	};

	class RenderVariableRwByteAddressBuffer final : public RenderVariableIOable
	{
	public:
		std::unique_ptr<RenderVariable> Clone() override
		{
			auto ret = MakeUniquePtr<RenderVariableRwByteAddressBuffer>();
			UnorderedAccessViewPtr val;
			this->Value(val);
			*ret = val;
			return ret;
		}

		void StreamIn([[maybe_unused]] RenderEffect const& effect, [[maybe_unused]] ResIdentifier& res) override
		{
			*this = UnorderedAccessViewPtr();
		}

#if ZENGINE_IS_DEV_PLATFORM
		void StreamOut([[maybe_unused]] std::ostream& os) const override
		{
		}
#endif

		RenderVariable& operator=(UnorderedAccessViewPtr const& value) override
		{
			val_ = value;
			return *this;
		}

		using RenderVariableIOable::operator=;

		void Value(UnorderedAccessViewPtr& val) const override
		{
			val = val_;
		}

		using RenderVariableIOable::Value;

	protected:
		UnorderedAccessViewPtr val_;
	};

	class RenderVariableStruct final : public RenderVariableArray<uint8_t>
	{
	public:
		explicit RenderVariableStruct(bool in_cbuff = false)
			: RenderVariableArray<uint8_t>(in_cbuff)
		{
		}

		RenderEffectStructType* StructType() const noexcept override
		{
			return struct_type_;
		}

		std::unique_ptr<RenderVariable> Clone() override
		{
			auto ret = RenderVariableArray<uint8_t>::Clone();

			auto* struct_var = checked_cast<RenderVariableStruct*>(ret.get());
			struct_var->struct_type_ = struct_type_;

			return ret;
		}

#if ZENGINE_IS_DEV_PLATFORM
        void Initialize(RenderEffect const& effect, std::string_view type)
        {
            struct_type_ = effect.StructTypeByName(type);
            if (!struct_type_) throw std::invalid_argument("Unknown EffectDesc struct: " + std::string(type));
        }

#endif

		void StreamIn(RenderEffect const& effect, ResIdentifier& res) override
		{
			struct_type_ = effect.StructTypeByName(ReadShortString(res));
			COMMON_ASSERT(struct_type_);
		}

#if ZENGINE_IS_DEV_PLATFORM
		void StreamOut(std::ostream& os) const override
		{
			WriteShortString(os, struct_type_->Name());
		}
#endif

		using RenderVariableArray<uint8_t>::operator=;
		using RenderVariableArray<uint8_t>::Value;

	protected:
		std::unique_ptr<RenderVariable> MakeInstance(bool in_cbuff) override
		{
			return MakeUniquePtr<RenderVariableStruct>(in_cbuff);
		}

	private:
		RenderEffectStructType* struct_type_ = nullptr;
	};

	std::unique_ptr<RenderVariableIOable> RenderVariableFactory(RenderEffectDataType type, bool is_array)
	{
		std::unique_ptr<RenderVariableIOable> ret;
		switch (type)
		{
		case REDT_bool:
			if (is_array)
			{
				ret = MakeUniquePtr<RenderVariableBoolArray>();
			}
			else
			{
				ret = MakeUniquePtr<RenderVariableBool>();
			}
			break;

		case REDT_uint:
			if (is_array)
			{
				ret = MakeUniquePtr<RenderVariableUIntArray>();
			}
			else
			{
				ret = MakeUniquePtr<RenderVariableUInt>();
			}
			break;

		case REDT_int:
			if (is_array)
			{
				ret = MakeUniquePtr<RenderVariableIntArray>();
			}
			else
			{
				ret = MakeUniquePtr<RenderVariableInt>();
			}
			break;

		case REDT_string:
			ret = MakeUniquePtr<RenderVariableString>();
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
			ret = MakeUniquePtr<RenderVariableTexture>();
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
			ret = MakeUniquePtr<RenderVariableRwTexture>();
			break;

		case REDT_sampler:
			ret = MakeUniquePtr<RenderVariableSampler>();
			break;

		case REDT_shader:
			ret = MakeUniquePtr<RenderVariableShader>();
			break;

		case REDT_float:
			if (is_array)
			{
				ret = MakeUniquePtr<RenderVariableFloatArray>();
			}
			else
			{
				ret = MakeUniquePtr<RenderVariableFloat>();
			}
			break;

		case REDT_uint2:
			if (is_array)
			{
				ret = MakeUniquePtr<RenderVariableUInt2Array>();
			}
			else
			{
				ret = MakeUniquePtr<RenderVariableUInt2>();
			}
			break;

		case REDT_uint3:
			if (is_array)
			{
				ret = MakeUniquePtr<RenderVariableUInt3Array>();
			}
			else
			{
				ret = MakeUniquePtr<RenderVariableUInt3>();
			}
			break;

		case REDT_uint4:
			if (is_array)
			{
				ret = MakeUniquePtr<RenderVariableUInt4Array>();
			}
			else
			{
				ret = MakeUniquePtr<RenderVariableUInt4>();
			}
			break;

		case REDT_int2:
			if (is_array)
			{
				ret = MakeUniquePtr<RenderVariableInt2Array>();
			}
			else
			{
				ret = MakeUniquePtr<RenderVariableInt2>();
			}
			break;

		case REDT_int3:
			if (is_array)
			{
				ret = MakeUniquePtr<RenderVariableInt3Array>();
			}
			else
			{
				ret = MakeUniquePtr<RenderVariableInt3>();
			}
			break;

		case REDT_int4:
			if (is_array)
			{
				ret = MakeUniquePtr<RenderVariableInt4Array>();
			}
			else
			{
				ret = MakeUniquePtr<RenderVariableInt4>();
			}
			break;

		case REDT_float2:
			if (is_array)
			{
				ret = MakeUniquePtr<RenderVariableFloat2Array>();
			}
			else
			{
				ret = MakeUniquePtr<RenderVariableFloat2>();
			}
			break;

		case REDT_float3:
			if (is_array)
			{
				ret = MakeUniquePtr<RenderVariableFloat3Array>();
			}
			else
			{
				ret = MakeUniquePtr<RenderVariableFloat3>();
			}
			break;

		case REDT_float4:
			if (is_array)
			{
				ret = MakeUniquePtr<RenderVariableFloat4Array>();
			}
			else
			{
				ret = MakeUniquePtr<RenderVariableFloat4>();
			}
			break;

		case REDT_float4x4:
			if (is_array)
			{
				ret = MakeUniquePtr<RenderVariableFloat4x4Array>();
			}
			else
			{
				ret = MakeUniquePtr<RenderVariableFloat4x4>();
			}
			break;

		case REDT_buffer:
		case REDT_structured_buffer:
		case REDT_consume_structured_buffer:
		case REDT_append_structured_buffer:
			ret = MakeUniquePtr<RenderVariableBuffer>();
			break;

		case REDT_rw_buffer:
		case REDT_rw_structured_buffer:
		case REDT_rasterizer_ordered_buffer:
		case REDT_rasterizer_ordered_structured_buffer:
			ret = MakeUniquePtr<RenderVariableRwBuffer>();
			break;

		case REDT_byte_address_buffer:
			ret = MakeUniquePtr<RenderVariableByteAddressBuffer>();
			break;

		case REDT_rw_byte_address_buffer:
		case REDT_rasterizer_ordered_byte_address_buffer:
			ret = MakeUniquePtr<RenderVariableRwByteAddressBuffer>();
			break;

		case REDT_struct:
			ret = MakeUniquePtr<RenderVariableStruct>();
			break;

		default:
			ZENGINE_UNREACHABLE("Invalid type");
		}

		return ret;
	}

#if ZENGINE_IS_DEV_PLATFORM
    template <typename T>
    void InitializeVariable(RenderVariable& variable, T const& value, uint32_t array_size)
    {
        if (array_size == 0)
            variable = value;
        else if constexpr (std::is_same_v<T, bool>)
            variable = std::vector<uint32_t>(array_size, value ? 1U : 0U);
        else
            variable = std::vector<T>(array_size, value);
    }

    std::unique_ptr<RenderVariable> CreateVariable(RenderEffect const& effect, EffectParameterDesc const& parameter)
    {
        auto const type = EffectType(parameter.type);
        auto value = RenderVariableFactory(type, parameter.array_size != 0);
        switch (type)
        {
        case REDT_bool: InitializeVariable(*value, false, parameter.array_size); break;
        case REDT_uint: InitializeVariable(*value, uint32_t(0), parameter.array_size); break;
        case REDT_int: InitializeVariable(*value, int32_t(0), parameter.array_size); break;
        case REDT_float: InitializeVariable(*value, 0.f, parameter.array_size); break;
        case REDT_uint2: InitializeVariable(*value, uint2(0, 0), parameter.array_size); break;
        case REDT_uint3: InitializeVariable(*value, uint3(0, 0, 0), parameter.array_size); break;
        case REDT_uint4: InitializeVariable(*value, uint4(0, 0, 0, 0), parameter.array_size); break;
        case REDT_int2: InitializeVariable(*value, int2(0, 0), parameter.array_size); break;
        case REDT_int3: InitializeVariable(*value, int3(0, 0, 0), parameter.array_size); break;
        case REDT_int4: InitializeVariable(*value, int4(0, 0, 0, 0), parameter.array_size); break;
        case REDT_float2: InitializeVariable(*value, float2(0, 0), parameter.array_size); break;
        case REDT_float3: InitializeVariable(*value, float3(0, 0, 0), parameter.array_size); break;
        case REDT_float4: InitializeVariable(*value, float4(0, 0, 0, 0), parameter.array_size); break;
        case REDT_float4x4:
            InitializeVariable(*value, parameter.array_size ? float4x4::Zero() : float4x4::Identity(), parameter.array_size);
            break;
        case REDT_struct:
            checked_cast<RenderVariableStruct*>(value.get())->Initialize(effect, parameter.struct_type);
            break;
        case REDT_texture2D:
        case REDT_textureCUBE:
            *value = TexturePtr();
            *value = std::string("float4");
            break;
        case REDT_sampler: break; // Created from SamplerStateDesc by the caller.
        default: ZENGINE_UNREACHABLE("Unsupported EffectDesc parameter");
        }
        std::visit([&](auto const& initial)
        {
            if constexpr (!std::is_same_v<std::decay_t<decltype(initial)>, std::monostate>)
                *value = initial;
        }, parameter.initial_value);
        return value;
    }

#endif

	std::unique_ptr<RenderVariable> StreamInVariable(
		RenderEffect const& effect, ResIdentifier& res, RenderEffectDataType type, uint32_t array_size)
	{
		auto ret = RenderVariableFactory(type, array_size != 0);
		ret->StreamIn(effect, res);
		return ret;
	}

#if ZENGINE_IS_DEV_PLATFORM
	void StreamOutVariable(std::ostream& os, RenderVariable const& var)
	{
		checked_cast<RenderVariableIOable const&>(var).StreamOut(os);
	}

#endif
} // namespace detail

	RenderVariable::RenderVariable() noexcept = default;
	RenderVariable::~RenderVariable() noexcept = default;

	RenderVariable& RenderVariable::operator=([[maybe_unused]] RenderVariable const& rhs)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] bool const & value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] uint32_t const & value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] int32_t const & value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] float const & value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] uint2 const & value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] uint3 const & value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] uint4 const & value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] int2 const & value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] int3 const & value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] int4 const & value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] float2 const & value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] float3 const & value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] float4 const & value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] float4x4 const & value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] TexturePtr const & value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] ShaderResourceViewPtr const & value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] UnorderedAccessViewPtr const & value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] SamplerStateObjectPtr const & value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] std::string const & value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] std::string_view value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] ShaderDesc const & value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] std::vector<bool> const & value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] std::vector<uint32_t> const & value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] std::vector<int32_t> const & value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] std::vector<float> const & value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] std::vector<uint2> const & value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] std::vector<uint3> const & value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] std::vector<uint4> const & value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] std::vector<int2> const & value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] std::vector<int3> const & value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] std::vector<int4> const & value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] std::vector<float2> const & value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] std::vector<float3> const & value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] std::vector<float4> const & value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] std::vector<float4x4> const & value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] std::span<bool const> value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] std::span<uint32_t const> value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] std::span<int32_t const> value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] std::span<float const> value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] std::span<uint2 const> value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] std::span<uint3 const> value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] std::span<uint4 const> value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] std::span<int2 const> value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] std::span<int3 const> value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] std::span<int4 const> value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] std::span<float2 const> value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] std::span<float3 const> value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] std::span<float4 const> value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] std::span<float4x4 const> value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] std::vector<uint8_t> const& value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	RenderVariable& RenderVariable::operator=([[maybe_unused]] std::span<uint8_t const> value)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	void RenderVariable::Value([[maybe_unused]] bool& value) const
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	void RenderVariable::Value([[maybe_unused]] uint32_t& value) const
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	void RenderVariable::Value([[maybe_unused]] int32_t& value) const
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	void RenderVariable::Value([[maybe_unused]] float& value) const
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	void RenderVariable::Value([[maybe_unused]] uint2& value) const
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	void RenderVariable::Value([[maybe_unused]] uint3& value) const
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	void RenderVariable::Value([[maybe_unused]] uint4& value) const
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	void RenderVariable::Value([[maybe_unused]] int2& value) const
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	void RenderVariable::Value([[maybe_unused]] int3& value) const
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	void RenderVariable::Value([[maybe_unused]] int4& value) const
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	void RenderVariable::Value([[maybe_unused]] float2& value) const
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	void RenderVariable::Value([[maybe_unused]] float3& value) const
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	void RenderVariable::Value([[maybe_unused]] float4& value) const
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	void RenderVariable::Value([[maybe_unused]] float4x4& value) const
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	void RenderVariable::Value([[maybe_unused]] TexturePtr& value) const
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	void RenderVariable::Value([[maybe_unused]] ShaderResourceViewPtr& value) const
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	void RenderVariable::Value([[maybe_unused]] UnorderedAccessViewPtr& value) const
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	void RenderVariable::Value([[maybe_unused]] SamplerStateObjectPtr& value) const
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	void RenderVariable::Value([[maybe_unused]] std::string& value) const
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	void RenderVariable::Value([[maybe_unused]] std::string_view& value) const
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	void RenderVariable::Value([[maybe_unused]] ShaderDesc& value) const
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	void RenderVariable::Value([[maybe_unused]] std::vector<bool>& value) const
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	void RenderVariable::Value([[maybe_unused]] std::vector<uint32_t>& value) const
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	void RenderVariable::Value([[maybe_unused]] std::vector<int32_t>& value) const
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	void RenderVariable::Value([[maybe_unused]] std::vector<float>& value) const
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	void RenderVariable::Value([[maybe_unused]] std::vector<uint2>& value) const
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	void RenderVariable::Value([[maybe_unused]] std::vector<uint3>& value) const
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	void RenderVariable::Value([[maybe_unused]] std::vector<uint4>& value) const
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	void RenderVariable::Value([[maybe_unused]] std::vector<int2>& value) const
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	void RenderVariable::Value([[maybe_unused]] std::vector<int3>& value) const
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	void RenderVariable::Value([[maybe_unused]] std::vector<int4>& value) const
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	void RenderVariable::Value([[maybe_unused]] std::vector<float2>& value) const
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	void RenderVariable::Value([[maybe_unused]] std::vector<float3>& value) const
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	void RenderVariable::Value([[maybe_unused]] std::vector<float4>& value) const
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	void RenderVariable::Value([[maybe_unused]] std::vector<float4x4>& value) const
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	void RenderVariable::Value([[maybe_unused]] std::vector<uint8_t>& value) const
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	void RenderVariable::BindToCBuffer([[maybe_unused]] RenderEffect const& effect, [[maybe_unused]] uint32_t cbuff_index,
		[[maybe_unused]] uint32_t offset, [[maybe_unused]] uint32_t stride)
	{
		ZENGINE_UNREACHABLE("Can't be called");
	}

	void RenderVariable::RebindToCBuffer([[maybe_unused]] RenderEffect const& effect, [[maybe_unused]] uint32_t cbuff_index)
	{		
		ZENGINE_UNREACHABLE("Can't be called");

	}
} // namespace RenderWorker
