#include "../Entity/Entity.h"
#include "../Context.h"
#include "../../Tool/UtilString.h"
#include "../../Util/util_string.h"
#include <boost/assert.hpp>
TypeInfo::TypeInfo(const std::string_view& strTypeName, const TypeInfo* pBaseType)
	:m_nTypeHash(UtilString::hash_value(strTypeName.data())),m_strTypeName(strTypeName), m_pBaseTypeInfo(pBaseType)
{

}

TypeInfo::~TypeInfo()
{

}

bool TypeInfo::IsTypeOf(uint32_t nHash) const
{
	const TypeInfo* current = this;
	while (current)
	{
		if (current->GetType() == nHash)
			return true;

		current = current->GetBaseTypeInfo();
	}

	return false;
}

bool TypeInfo::IsTypeOf(const TypeInfo* pTypeInfo) const
{
	const TypeInfo* current = this;
	while (current)
	{
		if (current == pTypeInfo)
			return true;

		current = current->GetBaseTypeInfo();
	}

	return false;
}

template<typename T>
bool TypeInfo::IsTypeOf() const
{
	return IsTypeOf(T::GetTypeInfoStatic());
}

IEntity::IEntity(Context* pContext)
	:m_pContext(pContext)
{
	m_Self = GetUniqueID();
	BOOST_ASSERT(m_pContext);

	std::shared_ptr<IEntity> pEntity(this);
	m_pContext->AddEntity(m_Self, pEntity);
}

IEntity::~IEntity()
{
	m_CPropDatas.Clear();
	m_pContext->RemoveEntity(m_Self);
}

bool IEntity::IsInstanceOf(uint32_t type) const
{
	return GetTypeInfo()->IsTypeOf(type);
}

bool IEntity::IsInstanceOf(const TypeInfo* typeInfo) const
{
	return GetTypeInfo()->IsTypeOf(typeInfo);
}

CPropData* IEntity::GetCustom() const
{
	return const_cast<CPropData*>(&m_CPropDatas);
}

PERSISTID IEntity::GetID() const
{
	return m_Self;
}

EnitityFactory::EnitityFactory(Context* pContext)
	:m_pContext(pContext)
{
	BOOST_ASSERT(m_pContext);
}
