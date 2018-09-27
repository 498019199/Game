#ifndef STX_ENTITY_H_
#define STX_ENTITY_H_
#pragma once

#include "../Core/predefine.h"
#include "../Core/Compenent/CPropData.h"
#include "../../Container/C++17/string_view.h"
#include <string>

class TypeInfo
{
public:
	TypeInfo(const std::string_view& strTypeName, const TypeInfo* pBaseType);
	~TypeInfo();
	bool IsTypeOf(uint32_t nHash) const;
	bool IsTypeOf(const TypeInfo* pTypeInfo)const;
	template<typename T> 
	bool IsTypeOf() const;

	uint32_t GetType() const{ return m_nTypeHash; }
	const std::string& GetTypeName() const { return m_strTypeName; }
	const TypeInfo* GetBaseTypeInfo() const { return m_pBaseTypeInfo; }
private:
	uint32_t m_nTypeHash;
	std::string m_strTypeName;
	const TypeInfo* m_pBaseTypeInfo;
};

#define STX_ENTITY(typeName, baseTypeName) \
   typedef typeName ClassName; \
        typedef baseTypeName BaseClassName; \
        virtual uint32_t GetType() const { return GetTypeInfoStatic()->GetType(); } \
        virtual const std::string& GetTypeName() const { return GetTypeInfoStatic()->GetTypeName(); } \
        virtual const TypeInfo* GetTypeInfo() const { return GetTypeInfoStatic(); } \
        static uint32_t GetTypeStatic() { return GetTypeInfoStatic()->GetType(); } \
        static const std::string& GetTypeNameStatic() { return GetTypeInfoStatic()->GetTypeName(); } \
        static const TypeInfo* GetTypeInfoStatic() { static const TypeInfo typeInfoStatic(#typeName, BaseClassName::GetTypeInfoStatic()); return &typeInfoStatic; } \

class IEntity
{
	friend class Context;
public:
	IEntity(Context* pContext);
	virtual ~IEntity();

	virtual uint32_t GetType() const = 0;
	virtual const std::string& GetTypeName() const = 0;
	virtual const TypeInfo* GetTypeInfo() const = 0;

	static const TypeInfo* GetTypeInfoStatic() { return 0; }
	bool IsInstanceOf(uint32_t type) const;
	bool IsInstanceOf(const TypeInfo* typeInfo) const;
	template<typename T> bool IsInstanceOf() const { return IsInstanceOf(T::GetTypeInfoStatic()); }
	template <class T> T* GetSubsystem() const;
	Context* GetContext() const { return m_pContext; }
	CPropData* GetCustom() const;

	PERSISTID GetID() const;
protected:
	Context * m_pContext;
	CPropData   m_CPropDatas;				// ¡Ÿ ±÷µ
	PERSISTID   m_Self;								// id
	//m_funcExecute;
};
template <class T> T* IEntity::GetSubsystem() const { return GetContext()->GetSubsystem<T>(); }

class IEntityEx:public IEntity
{
public:
	explicit IEntityEx(Context* context)
		:IEntity(context)
	{
	}

	virtual bool OnInit() = 0;
	virtual bool OnShut() = 0;
	virtual void Update() = 0;
};

class EnitityFactory
{
public:
	EnitityFactory(Context* pContext);
	virtual std::shared_ptr<IEntity> CreateObject() = 0;
	Context* GetContext() const { return m_pContext; }

	const TypeInfo* GetTypeInfo() const { return m_pTypeInfo; }
	uint32_t GetType() const { return m_pTypeInfo->GetType(); }
	const std::string& GetTypeName() const { return m_pTypeInfo->GetTypeName(); }

protected:
	Context * m_pContext;
	const TypeInfo* m_pTypeInfo;
};

template <typename T> 
class EnitityFactoryImpl : public EnitityFactory
{
public:
	explicit EnitityFactoryImpl(Context* context) 
		:EnitityFactory(context)
	{
		m_pTypeInfo = T::GetTypeInfoStatic();
	}

	IEntityPtr CreateObject() override
	{ 
		return std::static_pointer_cast<IEntity>(MakeSharedPtr<T>(m_pContext));
	}

	template<typename T, typename... Args>
	std::shared_ptr<T> CreateObjectArgs(Args&&... args)
	{
		return  MakeSharedPtr<T>(std::forward<Args>(args)...);
	}
};
#endif//STX_ENTITY_H_
