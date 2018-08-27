#ifndef IVarList_H
#define IVarList_H
#pragma once 
#include "persistid.h"
#include <string>
#include "../Container/C++17/string_view.h"
class IVarList
{
public:
	//是否为空
    virtual bool Empty() const = 0;
	//追加
    virtual void Append(const std::size_t count, const IVarList& s2) = 0;
	//获取个数
    virtual std::size_t GetCount() const = 0;
	//合并
    virtual void ConCat(IVarList& src) = 0;
	//获取类型
    virtual int GetType(std::size_t index) const = 0;
	//增加
	virtual void AddBool(const bool value) = 0;
    virtual void AddInt(const int value) = 0;
    virtual void AddInt64(const int64_t value) = 0;
    virtual void AddFloat(const float value) = 0;
    virtual void AddDouble(const double value) = 0;
    virtual void AddString(const char* value) = 0;
    virtual void AddWideString(const wchar_t* value) = 0;
    virtual void AddObject(const PERSISTID& obj) = 0;
	//获取
    virtual bool BoolVal(const std::size_t index) const = 0;
    virtual int IntVal(const std::size_t index) const = 0;
    virtual int64_t Int64Val(const std::size_t index) const = 0;
    virtual PERSISTID ObjectVal(const std::size_t index) const = 0;
    virtual double DoubleVal(const std::size_t index) const = 0;
    virtual float FloatVal(const std::size_t index) const = 0;
    virtual const char* StringVal(const std::size_t index) const = 0;
    virtual const wchar_t* WideStringVal(const std::size_t index) const = 0;
	//virtual float2 Float2Val(const std::size_t index) const = 0;
	//virtual float3 Float3Val(const std::size_t index) const = 0;
	//virtual float4 Float4Val(const std::size_t index) const = 0;
	//virtual float4x4 Float4x4Val(const std::size_t index) const = 0;

    IVarList& operator<<(const bool value)
	{
		AddBool(value);
		return *this;
	}

    IVarList& operator<<(const int value)
	{
		AddInt(value);
		return *this;
	}

    IVarList& operator<<(const float value)
	{
		AddFloat(value);
		return *this;
	}
    IVarList& operator<<(const double value)
	{
		AddDouble(value);
		return *this;
	}

    IVarList& operator<<(const char* value)
	{
		AddString(value);
		return *this;
	}

    IVarList& operator<<(const std::string& value)
	{
		AddString(value.c_str());
		return *this;
	}

    IVarList& operator<<(const wchar_t* value)
	{
		AddWideString(value);
		return *this;
	}

    IVarList&  operator<<(IVarList& value)
	{
		ConCat(value); 
		return *this;
	}
};
#endif //IVarList_H