// 2018年7月28日 移植klayGE里面的代码 zhangbei。
// Variable可变参数虚基类 
// VariableConcrete对Variable具体实现类，其他类型对应实例化，或继承实例化。VariableBool，VariableInt。。。
// CVarParameter 存储单个Variabl，或多个Variabl。
// CVarTemplate 存储Variabl数据类型
// CVarlist Variabl列表，可加载xml配置初始化
#ifndef _STX_TVAR_H1_
#define _STX_TVAR_H1_
#pragma once
#include "../Core/predefine.h"
#include <vector>
#include <string>

#include "../Math/Math.h"
#include "persistid.h"
#include "ArrayRef.hpp"

#include "../Container/C++17/string_view.h"
#include <boost/noncopyable.hpp>
#include <boost/assert.hpp>
enum CVarDataType
{
	CVAR_bool = 0,
	CVAR_string,
	CVAR_texture1D,
	CVAR_texture2D,
	CVAR_texture2DMS,
	CVAR_texture3D,
	CVAR_textureCUBE,
	CVAR_texture1DArray,
	CVAR_texture2DArray,
	CVAR_texture2DMSArray,
	CVAR_texture3DArray,
	CVAR_textureCUBEArray,
	CVAR_sampler,
	CVAR_shader,
	CVAR_uint,
	CVAR_uint2,
	CVAR_uint3,
	CVAR_uint4,
	CVAR_int,
	CVAR_int2,
	CVAR_int3,
	CVAR_int4,
	CVAR_float,
	CVAR_float2,
	CVAR_float2x2,
	CVAR_float2x3,
	CVAR_float2x4,
	CVAR_float3,
	CVAR_float3x2,
	CVAR_float3x3,
	CVAR_float3x4,
	CVAR_float4,
	CVAR_float4x2,
	CVAR_float4x3,
	CVAR_float4x4,
	CVAR_buffer,
	CVAR_structured_buffer,
	CVAR_byte_address_buffer,
	CVAR_rw_buffer,
	CVAR_rw_structured_buffer,
	CVAR_rw_texture1D,
	CVAR_rw_texture2D,
	CVAR_rw_texture3D,
	CVAR_rw_texture1DArray,
	CVAR_rw_texture2DArray,
	CVAR_rw_byte_address_buffer,
	CVAR_append_structured_buffer,
	CVAR_consume_structured_buffer,
	CVAR_rasterizer_ordered_buffer,
	CVAR_rasterizer_ordered_byte_address_buffer,
	CVAR_rasterizer_ordered_structured_buffer,
	CVAR_rasterizer_ordered_texture1D,
	CVAR_rasterizer_ordered_texture1DArray,
	CVAR_rasterizer_ordered_texture2D,
	CVAR_rasterizer_ordered_texture2DArray,
	CVAR_rasterizer_ordered_texture3D,

	CVAR_count,
};

struct TextureSubresource
{
	TexturePtr tex;
	uint32_t first_array_index;
	uint32_t num_items;
	uint32_t first_level;
	uint32_t num_levels;

	TextureSubresource()
	{
	}

	TextureSubresource(TexturePtr const & t, uint32_t fai, uint32_t ni, uint32_t fl, uint32_t nl)
		: tex(t), first_array_index(fai), num_items(ni), first_level(fl), num_levels(nl)
	{
	}
};

// 可变类型
class RenderVariable
{
public:
	RenderVariable();
	virtual ~RenderVariable() = 0;
	virtual RenderVariableUniPtr Clone() = 0;
	virtual RenderVariable& operator=(const bool& value);
	virtual RenderVariable& operator=(const uint32_t& value);
	virtual RenderVariable& operator=(const int32_t& value);
	virtual RenderVariable& operator=(const int64_t& value);
	virtual RenderVariable& operator=(const PERSISTID& value);
	virtual RenderVariable& operator=(const float& value);
	virtual RenderVariable& operator=(const uint2& value);
	virtual RenderVariable& operator=(const uint3& value);
	virtual RenderVariable& operator=(const uint4& value);
	virtual RenderVariable& operator=(const int2& value);
	virtual RenderVariable& operator=(const int3& value);
	virtual RenderVariable& operator=(const int4& value);
	virtual RenderVariable& operator=(const float2& value);
	virtual RenderVariable& operator=(const float3& value);
	virtual RenderVariable& operator=(const float4& value);
	virtual RenderVariable& operator=(const float4x4& value);
	virtual RenderVariable& operator=(const TexturePtr& value);
	virtual RenderVariable& operator=(const TextureSubresource& value);
	//virtual Variable& operator=(const SamplerStateObjectPtr& value);
	//virtual Variable& operator=(const GraphicsBufferPtr& value);
	virtual RenderVariable& operator=(const std::string& value);
	virtual RenderVariable& operator=(const std::wstring& value);
	//virtual Variable& operator=(const ShaderDesc& value);
	virtual RenderVariable& operator=(const std::vector<bool>& value);
	virtual RenderVariable& operator=(const std::vector<uint32_t>& value);
	virtual RenderVariable& operator=(const std::vector<int32_t>& value);
	virtual RenderVariable& operator=(const std::vector<float>& value);
	virtual RenderVariable& operator=(const std::vector<uint2>& value);
	virtual RenderVariable& operator=(const std::vector<uint3>& value);
	virtual RenderVariable& operator=(const std::vector<uint4>& value);
	virtual RenderVariable& operator=(const std::vector<int2>& value);
	virtual RenderVariable& operator=(const std::vector<int3>& value);
	virtual RenderVariable& operator=(const std::vector<int4>& value);
	virtual RenderVariable& operator=(const std::vector<float2>& value);
	virtual RenderVariable& operator=(const std::vector<float3>& value);
	virtual RenderVariable& operator=(const std::vector<float4>& value);
	virtual RenderVariable& operator=(const std::vector<float4x4>& value);

	virtual void Value(bool& val) const;
	virtual void Value(uint32_t& val) const;
	virtual void Value(int32_t& val) const;
	virtual void Value(int64_t& val) const;
	virtual void Value(PERSISTID& val) const;
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
	virtual void Value(TextureSubresource& val) const;
	//virtual void Value(SamplerStateObjectPtr& val) const;
	//virtual void Value(GraphicsBufferPtr& value) const;
	virtual void Value(std::string& val) const;
	virtual void Value(std::wstring& val) const;
	//virtual void Value(ShaderDesc& val) const;
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

	virtual void BindToCBuffer(RenderConstantBuffer& cbuff, uint32_t nOffset, uint32_t stride);
	virtual void RebindToCBuffer(RenderConstantBuffer& cbuff);
	virtual bool InCBuffer() const{return false;}
	virtual uint32_t CBufferOffset() const{return 0;}
	virtual uint32_t Stride() const{return 0;}
protected:
	struct CBufferDesc
	{
		RenderConstantBuffer* pBuff;
		uint32_t nOffset;
		uint32_t nStride;
	};
};

template <typename Ty>
class RenderVariableConcrete : public RenderVariable
{
public:
	RenderVariableConcrete(bool in_cbuff)
		: m_bIsBuff(false)
	{
		if (!m_bIsBuff)
		{
			new (m_Data.Val) Ty;
		}
	}
	RenderVariableConcrete()
		: RenderVariableConcrete(false)
	{
	}
	virtual ~RenderVariableConcrete()
	{
		if (!m_bIsBuff)
		{
			this->RetriveT().~Ty();
		}
	}

	std::unique_ptr<RenderVariable> Clone() override
	{
		auto ret = MakeUniquePtr<RenderVariableConcrete<Ty>>(m_bIsBuff);
		if (m_bIsBuff)
		{
			ret->m_Data = m_Data;
		}
		Ty val;
		this->Value(val);
		*ret = val;
		return std::move(ret);
	}

	virtual RenderVariable& operator=(const Ty& value) override
	{
		if (m_bIsBuff)
		{
			Ty& val_in_cbuff = *(m_Data.szBuffDesc.pBuff->template VariableInBuff<Ty>(m_Data.szBuffDesc.nOffset));
			if (val_in_cbuff != value)
			{
				val_in_cbuff = value;
				m_Data.szBuffDesc.pBuff->Dirty(true);
			}
		}
		else
		{
			this->RetriveT() = value;
		}
		return *this;
	}

	virtual void Value(Ty& val) const override
	{
		if (m_bIsBuff)
		{
			val = *(m_Data.szBuffDesc.pBuff->template VariableInBuff<Ty>(m_Data.szBuffDesc.nOffset));
		}
		else
		{
			val = this->RetriveT();
		}
	}

	virtual void BindToCBuffer(RenderConstantBuffer& cbuff, uint32_t nOffset, uint32_t stride) override
	{
		if (!m_bIsBuff)
		{
			Ty val = this->RetriveT();
			this->RetriveT().~Ty();
			m_bIsBuff = true;
			RenderVariable::CBufferDesc cbuff_desc;
			cbuff_desc.pBuff = &cbuff;
			cbuff_desc.nOffset = nOffset;
			cbuff_desc.nStride = stride;
			this->RetriveCBufferDesc() = std::move(cbuff_desc);
			this->operator=(val);
		}
	}

	virtual void RebindToCBuffer(RenderConstantBuffer& cbuff) override
	{
		BOOST_ASSERT(m_bIsBuff);
		m_Data.szBuffDesc.pBuff = &cbuff;
	}

	virtual bool InCBuffer() const override
	{
		return m_bIsBuff;
	}
	virtual uint32_t CBufferOffset() const override
	{
		return m_Data.szBuffDesc.nOffset;
	}
	virtual uint32_t Stride() const override
	{
		return m_Data.szBuffDesc.nStride;
	}

protected:
	Ty & RetriveT()
	{
		union Raw2T
		{
			uint8_t* raw;
			Ty* t;
		} r2t;
		r2t.raw = m_Data.Val;
		return *r2t.t;
	}
	const Ty& RetriveT() const
	{
		union Raw2T
		{
			const uint8_t* raw;
			const Ty* t;
		} r2t;
		r2t.raw = m_Data.Val;
		return *r2t.t;
	}
	CBufferDesc& RetriveCBufferDesc()
	{
		return m_Data.szBuffDesc;
	}
	CBufferDesc const & RetriveCBufferDesc() const
	{
		return m_Data.szBuffDesc;
	}
protected:
	bool m_bIsBuff;
	union VarData
	{
		CBufferDesc szBuffDesc;
		uint8_t Val[sizeof(Ty)];
	};
	VarData m_Data;
};

class RenderVariableFloat4x4 : public RenderVariableConcrete<float4x4>
{
public:
	explicit RenderVariableFloat4x4(bool in_cbuff);
	RenderVariableFloat4x4();

	std::unique_ptr<RenderVariable> Clone() override;

	virtual RenderVariable& operator=(float4x4 const & value) override;
	virtual void Value(float4x4& val) const override;
};

template <typename T>
class RenderVariableArray : public RenderVariableConcrete<std::vector<T>>
{
public:
	explicit RenderVariableArray(bool in_cbuff)
		: RenderVariableConcrete<std::vector<T>>(in_cbuff)
	{
	}
	RenderVariableArray()
		: RenderVariableConcrete<std::vector<T>>()
	{
	}

	std::unique_ptr<RenderVariable> Clone() override
	{
		auto ret = MakeUniquePtr<RenderVariableArray<T>>(this->m_bIsBuff);
		if (this->m_bIsBuff)
		{
			ret->RenderVariableConcrete<std::vector<T>>::m_Data = this->m_Data;
			ret->m_Size = this->m_Size;

			auto const & src_cbuff_desc = this->RetriveCBufferDesc();
			uint8_t const * src = src_cbuff_desc.pBuff->template VariableInBuff<uint8_t>(src_cbuff_desc.nOffset);

			auto const & dst_cbuff_desc = ret->RetriveCBufferDesc();
			uint8_t* dst = dst_cbuff_desc.pBuff->template VariableInBuff<uint8_t>(dst_cbuff_desc.nOffset);

			for (size_t i = 0; i < m_Size; ++i)
			{
				*reinterpret_cast<T*>(dst) = *reinterpret_cast<T const *>(src);
				src += src_cbuff_desc.nStride;
				dst += dst_cbuff_desc.nStride;
			}

			dst_cbuff_desc.pBuff->Dirty(true);
		}
		else
		{
			ret->RetriveT() = this->RetriveT();
		}
		return std::move(ret);
	}

	virtual RenderVariable& operator=(std::vector<T> const & value) override
	{
		if (this->m_bIsBuff)
		{
			uint8_t const * src = reinterpret_cast<uint8_t const *>(value.data());

			auto& cbuff_desc = this->RetriveCBufferDesc();
			uint8_t* dst = cbuff_desc.pBuff->template VariableInBuff<uint8_t>(cbuff_desc.nOffset);

			m_Size = static_cast<uint32_t>(value.size());
			for (size_t i = 0; i < value.size(); ++i)
			{
				*reinterpret_cast<T*>(dst) = *reinterpret_cast<T const *>(src);
				src += sizeof(T);
				dst += cbuff_desc.nStride;
			}

			cbuff_desc.pBuff->Dirty(true);
		}
		else
		{
			this->RetriveT() = value;
		}
		return *this;
	}

	virtual void Value(std::vector<T>& val) const override
	{
		if (this->m_bIsBuff)
		{
			auto const & cbuff_desc = this->RetriveCBufferDesc();
			uint8_t const * src = cbuff_desc.pBuff->template VariableInBuff<uint8_t>(cbuff_desc.nOffset);

			val.resize(m_Size);

			for (size_t i = 0; i < m_Size; ++i)
			{
				val[i] = *reinterpret_cast<T const *>(src);
				src += cbuff_desc.nStride;
			}
		}
		else
		{
			val = this->RetriveT();
		}
	}

private:
	uint32_t m_Size;
};

class RenderVariableFloat4x4Array : public RenderVariableConcrete<std::vector<float4x4>>
{
public:
	explicit RenderVariableFloat4x4Array(bool in_cbuff);
	RenderVariableFloat4x4Array();

	std::unique_ptr<RenderVariable> Clone() override;

	virtual RenderVariable& operator=(const std::vector<float4x4>& value) override;
	virtual void Value(std::vector<float4x4>& val) const override;

private:
	uint32_t m_Size;
};

class VariableTexture : public RenderVariable
{
public:
	std::unique_ptr<RenderVariable> Clone() override;

	virtual RenderVariable& operator=(const TexturePtr  & value);
	virtual RenderVariable& operator=(const TextureSubresource  & value);
	virtual RenderVariable& operator=(const std::string  & value);

	virtual void Value(TexturePtr& val) const;
	virtual void Value(TextureSubresource& val) const;
	virtual void Value(std::string& val) const;

protected:
	mutable TextureSubresource val_;
	std::string elem_type_;
};

class RenderVariableBuffer : public RenderVariable
{
public:
	std::unique_ptr<RenderVariable> Clone() override;

	virtual RenderVariable& operator=(DataBufferPtr const & value);
	virtual RenderVariable& operator=(std::string const & value);

	virtual void Value(DataBufferPtr& val) const;
	virtual void Value(std::string& val) const;

protected:
	DataBufferPtr val_;
	std::string elem_type_;
};

class RenderVariableByteAddressBuffer : public RenderVariable
{
public:
	std::unique_ptr<RenderVariable> Clone() override;

virtual RenderVariable& operator=(DataBufferPtr const & value);
	virtual RenderVariable& operator=(std::string const & value);

	virtual void Value(DataBufferPtr& val) const;
	virtual void Value(std::string& val) const;

protected:
	DataBufferPtr m_Val;
	std::string m_Type;
};

typedef RenderVariableConcrete<bool> RenderVariableBool;
typedef RenderVariableConcrete<uint32_t> RenderVariableUInt;
typedef RenderVariableConcrete<int32_t> RenderVariableInt;
typedef RenderVariableConcrete<int64_t> RenderVariableInt64;
typedef RenderVariableConcrete<float> RenderVariableFloat;
typedef RenderVariableConcrete<uint2> RenderVariableUInt2;
typedef RenderVariableConcrete<uint3> RenderVariableUInt3;
typedef RenderVariableConcrete<uint4> RenderVariableUInt4;
typedef RenderVariableConcrete<int2> RenderVariableInt2;
typedef RenderVariableConcrete<int3> RenderVariableInt3;
typedef RenderVariableConcrete<int4> RenderVariableInt4;
typedef RenderVariableConcrete<float2> RenderVariableFloat2;
typedef RenderVariableConcrete<float3> RenderVariableFloat3;
typedef RenderVariableConcrete<float4> RenderVariableFloat4;
//typedef VariableConcrete<SamplerStateObjectPtr> VariableSampler;
typedef RenderVariableConcrete<std::string> RenderVariableString;
typedef RenderVariableConcrete<std::wstring> RenderVariableWString;
typedef RenderVariableConcrete<PERSISTID> RenderVariableOBJ;
//typedef VariableConcrete<ShaderDesc> VariableShader;
typedef RenderVariableArray<bool> RenderVariableBoolArray;
typedef RenderVariableArray<uint32_t> RenderVariableUIntArray;
typedef RenderVariableArray<int32_t> RenderVariableIntArray;
typedef RenderVariableArray<float> RenderVariableFloatArray;
typedef RenderVariableArray<int2> RenderVariableInt2Array;
typedef RenderVariableArray<int3> RenderVariableInt3Array;
typedef RenderVariableArray<int4> RenderVariableInt4Array;
typedef RenderVariableArray<float2> RenderVariableFloat2Array;
typedef RenderVariableArray<float3> RenderVariableFloat3Array;
typedef RenderVariableArray<float4> RenderVariableFloat4Array;

class RenderConstantBuffer : boost::noncopyable
{
public:
	RenderConstantBuffer()
		: m_bDirty(true)
	{
	}

	void Load(std::string const & name);
	//void StreamIn(ResIdentifierPtr const & res);
	//void StreamOut(std::ostream& os) const;
	std::unique_ptr<RenderConstantBuffer> Clone(RenderCVarlist& src_list, RenderCVarlist& dst_list);

	std::string const & Name() const
	{
		return m_HashName->first;
	}
	size_t NameHash() const
	{
		return m_HashName->second;
	}

	void AddParameter(uint32_t index);

	uint32_t NumParameters() const
	{
		return m_ParamIndexVec ? static_cast<uint32_t>(m_ParamIndexVec->size()) : 0;
	}
	uint32_t ParameterIndex(uint32_t index) const
	{
		return (*m_ParamIndexVec)[index];
	}

	void Resize(uint32_t size);

	template <typename T>
	T const * VariableInBuff(uint32_t nOffset) const
	{
		union Raw2T
		{
			uint8_t const * raw;
			T const * t;
		} r2t;
		r2t.raw = &m_buff[nOffset];
		return r2t.t;
	}
	template <typename T>
	T* VariableInBuff(uint32_t nOffset)
	{
		union Raw2T
		{
			uint8_t* raw;
			T* t;
		} r2t;
		r2t.raw = &m_buff[nOffset];
		return r2t.t;
	}

	void Dirty(bool dirty)
	{
		m_bDirty = dirty;
	}
	bool Dirty() const
	{
		return m_bDirty;
	}

	void Update();
	const DataBufferPtr& HWBuff() const
	{
		return m_DataBuffPtr;
	}
	void BindHWBuff(const DataBufferPtr& buff);
private:
	std::shared_ptr<std::pair<std::string, size_t>> m_HashName;
	std::shared_ptr<std::vector<uint32_t>> m_ParamIndexVec;
	DataBufferPtr m_DataBuffPtr;
	std::vector<uint8_t> m_buff;
	bool m_bDirty;
};

// 存储图片数据
class RenderVariableAnnotation : boost::noncopyable
{
public:
	void Load(XMLNodePtr const & node);
	void StreamIn(ResIdentifierPtr const & res);
	void StreamOut(std::ostream& os) const;


	uint32_t Type() const
	{
		return m_nType;
	}
	std::string const & Name() const
	{
		return m_strName;
	}

	template <typename T>
	void Value(T& val) const
	{
		m_ImageData->Value(val);
	}

private:
	uint32_t m_nType;
	std::string m_strName;
	std::unique_ptr<RenderVariable> m_ImageData;
};

// 存储单个数据
class RenderCVarParameter : boost::noncopyable
{
public:
	void Load(XMLNodePtr const & node);

//	void StreamIn(ResIdentifierPtr const & res);
//#if KLAYGE_IS_DEV_PLATFORM
//	void StreamOut(std::ostream& os) const;
//#endif

	std::unique_ptr<RenderCVarParameter> Clone();

	uint32_t Type() const
	{
		return m_Type;
	}

	RenderVariable const & QueryVar() const
	{
		BOOST_ASSERT(m_Var);
		return *m_Var;
	}

	std::shared_ptr<std::string> const & ArraySize() const
	{
		return m_ArraySize;
	}

	std::string const & Name() const
	{
		return m_NameHash->first;
	}
	size_t NameHash() const
	{
		return m_NameHash->second;
	}
	bool HasSemantic() const
	{
		return !!m_SemanticHash;
	}
	std::string const & Semantic() const;
	size_t SemanticHash() const;

	uint32_t NumAnnotations() const;
	RenderVariableAnnotation const & Annotation(uint32_t n) const;


	template <typename T>
	RenderCVarParameter& operator=(T const & value)
	{
		*m_Var = value;
		return *this;
	}

	template <typename T>
	void Value(T& val) const
	{
		m_Var->Value(val);
	}

	void BindToCBuffer(RenderConstantBuffer& cbuff, uint32_t offset, uint32_t stride);
	void RebindToCBuffer(RenderConstantBuffer& cbuff);
	RenderConstantBuffer& CBuffer() const
	{
		BOOST_ASSERT(m_pBuff);
		return *m_pBuff;
	}
	bool InCBuffer() const
	{
		return m_Var->InCBuffer();
	}
	uint32_t CBufferOffset() const
	{
		return m_Var->CBufferOffset();
	}
	uint32_t Stride() const
	{
		return m_Var->Stride();
	}
	template <typename T>
	T const * MemoryInCBuff() const
	{
		return m_pBuff->VariableInBuff<T>(m_Var->CBufferOffset());
	}
	template <typename T>
	T* MemoryInCBuff()
	{
		return m_pBuff->VariableInBuff<T>(m_Var->CBufferOffset());
	}

private:
	std::shared_ptr<std::pair<std::string, size_t>> m_NameHash;
	std::shared_ptr<std::pair<std::string, size_t>> m_SemanticHash;
	uint32_t m_Type;
	std::unique_ptr<RenderVariable> m_Var;
	std::shared_ptr<std::string> m_ArraySize;
	std::shared_ptr<std::vector<std::unique_ptr<RenderVariableAnnotation>>> m_ImageListData;
	RenderConstantBuffer* m_pBuff;
};

// 对CVarlist扩充
class RenderCVarlist : boost::noncopyable
{
	friend class RenderCVarTemplate;
public:
	void Load(ArrayRef<std::string> names);

	RenderCVarlistPtr Clone();

	std::string const & ResName() const;
	size_t ResNameHash() const;
	uint32_t NumParameters() const;

	RenderCVarParameter* QueryBySemantic(std::string_view semantic) const;
	RenderCVarParameter* QueryByName(std::string_view name) const;
	RenderCVarParameter* QueryByIndex(uint32_t n) const;

	uint32_t NumCBuffers() const;
	RenderConstantBuffer* CBufferByName(std::string_view name) const;
	RenderConstantBuffer* CBufferByIndex(uint32_t n) const;

	uint32_t NumMacros() const;
	std::pair<std::string, std::string> const & MacroByIndex(uint32_t n) const;
private:
	// 数据对应的类型
	RenderCVarTemplatePtr m_DataTemplate;
	// 记录CVarDataType标准数据
	std::vector<std::unique_ptr<RenderCVarParameter>> m_ParamVar;
	// 记录数据缓存
	std::vector<std::unique_ptr<RenderConstantBuffer>> m_Buffer;
};

// 数据类型记录
class RenderCVarTemplate : boost::noncopyable
{
public:
	void Load(ArrayRef<std::string> names, RenderCVarlist& effect);
	bool StreamIn(ResIdentifierPtr const & source, RenderCVarlist& effect);

	std::string const & ResName() const{return m_strNameRes;}
	size_t ResNameHash() const{return m_NameHash;}
	uint32_t NumMacros() const{return static_cast<uint32_t>(m_Typs.size());}
	std::pair<std::string, std::string> const & MacroByIndex(uint32_t n) const;

private:
	void PreprocessIncludes(XMLDocument& doc, XMLNode& root, std::vector<std::unique_ptr<XMLDocument>>& include_docs);
	void RecursiveIncludeNode(XMLNode const & root, std::vector<std::string>& FileNames) const;
	void InsertIncludeNodes(XMLDocument& target_doc, XMLNode& target_root,
		XMLNodePtr const & target_place, XMLNode const & include_root) const;
	XMLNodePtr ResolveInheritTechNode(XMLDocument& doc, XMLNode& root, XMLNodePtr const & tech_node);
	void ResolveOverrideTechs(XMLDocument& doc, XMLNode& root);
	void Load(XMLNode const & root, RenderCVarlist& cvlist);
private:
	uint64_t m_nTimeMap;
	std::string m_strNameRes;
	size_t m_NameHash;
	std::vector<std::pair<std::pair<std::string, std::string>, bool>> m_Typs;
};

// 加载文件接口
RenderCVarlistPtr SyncLoadRenderEffect(std::string const & file_name);
RenderCVarlistPtr SyncLoadRenderEffects(ArrayRef<std::string> file_names);
RenderCVarlistPtr ASyncLoadRenderEffect(std::string const & file_name);
RenderCVarlistPtr ASyncLoadRenderEffects(ArrayRef<std::string> file_names);
#endif//_STX_TVAR_H1_