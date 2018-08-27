#ifndef TVarList_H
#define TVarList_H
#pragma once 

#include "../Container/macro.h"
#include "../Container/var_type.h"
#include "../Container/i_varlist.h"
#include "../Container/persistid.h"
#include "../Container/momery.h"
#include "../Container/String.h"
#include "../Math/Math.h"
#include "cvar_list.h"
#include "../Tool/UtilString.h"
#include <memory>

class Variable
{
public:
	Variable();
	virtual ~Variable();

	virtual Variable& operator=(const bool& value);
	virtual Variable& operator=(const int& value);
	virtual Variable& operator=(const int64_t& value);
	virtual Variable& operator=(const float& value);
	virtual Variable& operator=(const double& value);
	virtual Variable& operator=(const String& value);
	virtual Variable& operator=(const WString& value);
	virtual Variable& operator=(const PERSISTID& value);
	virtual Variable& operator=(const float2& value);
	virtual Variable& operator=(const float3& value);
	virtual Variable& operator=(const float4& value);
	virtual Variable& operator=(const float4x4& value);

	virtual int GetType() const;
	virtual void Value(bool& val) const;
	virtual void Value(int& val) const;
	virtual void Value(int64_t& val) const;
	virtual void Value(float& val) const;
	virtual void Value(double& val) const;
	virtual void Value(String& val) const;
	virtual void Value(WString& val) const;
	virtual void Value(PERSISTID& val) const;
	virtual void Value(float2& val) const;
	virtual void Value(float3& val) const;
	virtual void Value(float4& val) const;
	virtual void Value(float4x4& val) const;
};

template<typename T>
class VariableCrete :public Variable
{
public:
	VariableCrete()
		:m_Type(TYPE_NONE)
	{
		new (m_Var) T;
	}

	~VariableCrete()
	{
		this->RetriveT().~T();
	}

	virtual Variable& operator=(const T& value) override
	{
		this->RetriveT() = value;
		return *this;
	}

	virtual void Value(T& val) const override
	{
		val = this->RetriveT();
	}
	
	void SetType(int nType)
	{
		m_Type = nType;
	}
	
	virtual int GetType()const override
	{
		return m_Type;
	}
private:
	T & RetriveT()
	{
		union Raw2T
		{
			uint8_t* raw;
			T* t;
		} r2t;
		r2t.raw = m_Var;
		return *r2t.t;
	}
	const T& RetriveT() const
	{
		union Raw2T
		{
			const uint8_t* raw;
			const T* t;
		} r2t;
		r2t.raw = m_Var;
		return *r2t.t;
	}
private:
	uint8_t m_Type;
	uint8_t m_Var[sizeof(T)];
};

typedef VariableCrete<bool> VariableBool;
typedef VariableCrete<int> VariableInt;
typedef VariableCrete<int64_t> VariableInt64;
typedef VariableCrete<float> VariableFloat;
typedef VariableCrete<double> VariableDouble;
typedef VariableCrete<String> VariableString;
typedef VariableCrete<WString> VariableWString;
typedef VariableCrete<PERSISTID> VariablePERSISTID;
typedef VariableCrete<float2> VariableFloat2;
typedef VariableCrete<float3> VariableFloat3;
typedef VariableCrete<float4> VariableFloat4;
typedef VariableCrete<float4x4> VariableFloat4x4;
typedef std::shared_ptr<Variable> VariablePtr;

template<uint32_t SIZE, uint32_t BUFFER_SIZE>
class TVarList :public IVarList
{
	typedef std::vector<Variable> DetailType;
public:
	typedef typename DetailType::value_type value_type;
	typedef value_type* pointer;
	typedef  value_type const * const_pointer;

	typedef typename DetailType::reference reference;
	typedef typename DetailType::const_reference const_reference;

	typedef typename DetailType::iterator iterator;
	typedef typename DetailType::const_iterator const_iterator;

	typedef typename DetailType::size_type size_type;
	typedef typename DetailType::difference_type difference_type;
public:
	TVarList()
	{
		m_VarList.resize(SIZE);
		m_Size = SIZE;
		m_Capacity = 0;

		m_pBuffer = m_Buffer;
		m_nBufferSize = BUFFER_SIZE;
		m_nBuffUsed = 0;
	}

	~TVarList()
	{
		if (m_nBufferSize > BUFFER_SIZE)
		{
			delete [] m_pBuffer;
		}
	}

	iterator begin() noexcept
	{
		return m_VarList.begin();
	}
	constexpr const_iterator begin() const noexcept
	{
		return m_VarList.begin();
	}
	iterator end() noexcept
	{
		return m_VarList.end();
	}
	constexpr const_iterator end() const noexcept
	{
		return m_VarList.end();
	}
	reference operator[](size_type off) noexcept
	{
		return m_VarList[off];
	}
	constexpr const_reference operator[](size_type off) const noexcept
	{
		return m_VarList[off];
	}

	void ConCat(IVarList& src)
	{
		inner_append(src, 0, src.GetCount());
	}

	void Append(const std::size_t count, const IVarList& s2)
	{
		inner_append(s2, 0, count);
	}

	std::size_t GetCount() const
	{
		return m_Capacity;
	}

	int GetType(std::size_t index) const
	{
		if (index > m_Capacity)
		{
			return false;
		}

		return m_VarList[index]->GetType();
	}

	bool Empty() const
	{
		return 0 == m_Capacity;
	}

	void Clear()
	{
		m_Capacity = 0;
		m_Size = 0;
	}

	void AddBool(const bool value)
	{
		auto val = MakeSharedPtr<VariableBool>();
		*val = value;
		val->SetType(CType_bool);
		AddVarData(val);
	}

	void AddInt(const int value)
	{
		auto val = MakeSharedPtr<VariableInt>();
		*val = value;
		val->SetType(CType_int);
		AddVarData(val);
	}

	void AddInt64(const int64_t value)
	{
		auto val = MakeSharedPtr<VariableInt64>();
		*val = value;
		val->SetType(CType_int64);
		AddVarData(val);
	}

	void AddFloat(const float value)
	{
		auto val = MakeSharedPtr<VariableFloat>();
		*val = value;
		val->SetType(CType_float);
		AddVarData(val);
	}

	void AddDouble(const double value)
	{
		auto val = MakeSharedPtr<VariableDouble>();
		*val = value;
		val->SetType(CType_double);
		AddVarData(val);
	}

	void AddString(const char* value)
	{
		int nSize = strlen(value) + 1;
		char *str = AddBufferData(nSize);
		memcpy(str, value, nSize);
		
		auto val = MakeSharedPtr<VariableInt>();
		val->SetType(CType_int64);
		*val = m_nBuffUsed;
		m_nBuffUsed += nSize;
		AddVarData(val);
	}

	void AddWideString(const wchar_t* value)
	{
		int nSize = (wcslen(value) + 1) * sizeof(wchar_t);
		wchar_t *str = (wchar_t*)AddBufferData(nSize);
		memcpy(str, value, nSize);

		auto val = MakeSharedPtr<VariableInt>();
		val->SetType(CType_int64);
		*val = m_nBuffUsed;
		m_nBuffUsed += nSize;
		AddVarData(val);
	}

	void AddObject(const PERSISTID& value)
	{
		auto val = MakeSharedPtr<VariablePERSISTID>();
		*val = value;
		val->SetType(CType_object);
		AddVarData(val);
	}

	void AddFloat2(const float2& value)
	{
		auto val = MakeSharedPtr<VariableFloat2>();
		*val = value;
		val->SetType(CType_float2);
		AddVarData(val);
	}

	void AddFloat3(const float3& value)
	{
		auto val = MakeSharedPtr<VariableFloat3>();
		*val = value;
		val->SetType(CType_float3);
		AddVarData(val);
	}

	void AddFloat4(const float4& value)
	{
		auto val = MakeSharedPtr<VariableFloat4>();
		*val = value;
		val->SetType(CType_float);
		AddVarData(val);
	}

	void AddFloat4x4(const float4x4& value)
	{
		auto val = MakeSharedPtr<VariableFloat4x4>();
		*val = value;
		val->SetType(CType_float4x4);
		AddVarData(val);
	}

	bool BoolVal(const std::size_t index) const
	{
		if (index > m_Capacity)
		{
			return false;
		}

		bool tmp;
		m_VarList[index]->Value(tmp);
		return tmp;
	}

	int IntVal(const std::size_t index)const
	{
		if (index > m_Capacity)
		{
			return false;
		}

		int tmp;
		m_VarList[index]->Value(tmp);
		return tmp;
	}

	int64_t Int64Val(const std::size_t index)const
	{
		if (index > m_Capacity)
		{
			return false;
		}

		int64_t tmp;
		m_VarList[index]->Value(tmp);
		return tmp;
	}

	float FloatVal(const std::size_t index)const
	{
		if (index > m_Capacity)
		{
			return false;
		}

		float tmp;
		m_VarList[index]->Value(tmp);
		return tmp;
	}

	double DoubleVal(const std::size_t index)const
	{
		if (index > m_Capacity)
		{
			return false;
		}

		double tmp;
		m_VarList[index]->Value(tmp);
		return tmp;
	}

	const char* StringVal(const std::size_t index)const
	{
		if (index > m_Capacity)
		{
			return false;
		}

		int tmp;
		m_VarList[index]->Value(tmp);
		return reinterpret_cast<char*>(m_pBuffer + tmp);
	}

	const wchar_t* WideStringVal(const std::size_t index)const
	{
		if (index > m_Capacity)
		{
			return false;
		}

		int tmp;
		m_VarList[index]->Value(tmp);
		return reinterpret_cast<wchar_t*>(m_pBuffer + tmp);
	}

	PERSISTID ObjectVal(const std::size_t index)const
	{
		if (index > m_Capacity)
		{
			return false;
		}

		PERSISTID tmp;
		m_VarList[index]->Value(tmp);
		return tmp;
	}

	float FloatVal2(const std::size_t index)const
	{
		if (index > m_Capacity)
		{
			return false;
		}

		float2 tmp;
		m_VarList[index]->Value(tmp);
		return tmp;
	}

	float FloatVal3(const std::size_t index)const
	{
		if (index > m_Capacity)
		{
			return false;
		}

		float3 tmp;
		m_VarList[index]->Value(tmp);
		return tmp;
	}

	float FloatVal4(const std::size_t index)const
	{
		if (index > m_Capacity)
		{
			return false;
		}

		float4 tmp;
		m_VarList[index]->Value(tmp);
		return tmp;
	}

	float FloatVal4x4(const std::size_t index)const
	{
		if (index > m_Capacity)
		{
			return false;
		}

		float4x4 tmp;
		m_VarList[index]->Value(tmp);
		return tmp;
	}

private:
	char* AddBufferData(std::size_t nSize)
	{
		if (m_nBuffUsed + nSize >= m_nBufferSize)
		{
			std::size_t new_size = m_nBufferSize * 2;
			char* p = NEW char[new_size];
			memcpy(p, m_Buffer, m_nBufferSize);
			if (m_nBufferSize > BUFFER_SIZE)
			{
				delete[] m_pBuffer;
			}
			m_pBuffer = p;
			m_nBufferSize = new_size;
		}
		return m_pBuffer + m_nBuffUsed;
	}

	void AddVarData(VariablePtr val)
	{
		if (m_Size < m_Capacity)
		{
			uint32_t nNewSize = m_Size * 2;
			m_VarList.resize(nNewSize);
		}

		m_VarList[m_Capacity] = std::move(val);
		m_Capacity++;
	}

	void inner_append(const IVarList& src, std::size_t index, std::size_t size)
	{
		for (std::size_t i = 0; i < size; ++i)
		{
			switch (src.GetType(i))
			{
			case CType_bool:
				AddBool(src.BoolVal(i));
				break;
			case CType_int:
				AddInt(src.IntVal(i));
				break;
			case CType_float:
				AddFloat(src.FloatVal(i));
				break;
			case CType_double:
				AddDouble(src.DoubleVal(i));
				break;
			case CType_string:
				AddString(src.StringVal(i));
				break;
			case CType_widestring:
				AddWideString(src.WideStringVal(i));
				break;
			}
		}
	}
private:
	std::vector<VariablePtr> m_VarList;
	uint32_t m_Size;
	uint32_t m_Capacity;
	char m_Buffer[BUFFER_SIZE];
	char* m_pBuffer;
	std::size_t m_nBufferSize;
	std::size_t m_nBuffUsed;
};

typedef TVarList<8,128> CVarList;
#endif //TVarList_H
