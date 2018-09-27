#include "Context.h"
#include "../Util/UtilTool.h"

std::unique_ptr<Context> Context::m_InstanceContext;
Context* Context::Instance()
{
	if (!m_InstanceContext)
	{
		if (!m_InstanceContext)
		{
			m_InstanceContext = MakeUniquePtr<Context>();
		}
	}
	return m_InstanceContext.get();
}

void Context::SetQuit()
{
	m_Quit = true;
}

bool Context::GetQuit() const
{
	return m_Quit;
}

void Context::Close()
{
	EndCore();
}

bool Context::FindGlobalValue(const char* szName)
{
	auto it = m_GlobalVar.find(szName);
	if (m_GlobalVar.end() == it)
	{
		return false;
	}

	return true;
}

int Context::GetGlobalCount()
{
	return m_GlobalVar.size();
}

bool Context::SetGlobalValue(const char* szName, VariablePtr val)
{
	auto it = m_GlobalVar.find(szName);
	if (m_GlobalVar.end() == it)
	{
		m_GlobalVar.assign(szName, val);
		return true;
	}

	it.get_data() = std::move(val);
	return true;
}

VariablePtr Context::GetGlobalValue(const char* szName)
{
	auto it = m_GlobalVar.find(szName);
	return it.get_data();
}

bool Context::RemoveGlobalValue(const char* szName)
{
	auto it = m_GlobalVar.find(szName);
	if (m_GlobalVar.end() == it)
	{
		return true;
	}

	m_GlobalVar.eraser(szName);
	return true;
}

void Context::DisPlay(float fTimer)
{
	hash_list<char, std::shared_ptr<IEntityEx>>::iterator it = m_SubSystemMrg.begin();
	for (; it != m_SubSystemMrg.end(); ++it)
	{
		it.get_data()->Update();
	}
}

void Context::RegisterFactory(EnitityFactory* factory)
{
	if (nullptr == factory)
	{
		return;
	}

	std::shared_ptr<EnitityFactory> pFactoryPtr(factory);
	m_FactoryMrg.assign(factory->GetTypeName().c_str(), pFactoryPtr);
}

std::shared_ptr<IEntity> Context::CreateObject(const char* szName)
{
	auto it = m_FactoryMrg.find(szName);
	return it.get_data()->CreateObject();
}

void Context::RegisterSubsystem(IEntityEx* pEntity)
{
	if (nullptr == pEntity)
	{
		return;
	}

	pEntity->OnInit();
	std::shared_ptr<IEntityEx> pEntityPtr(pEntity);
	m_SubSystemMrg.assign(pEntity->GetTypeName().c_str(), pEntityPtr);
}

bool Context::RemoveSubsystem(const char* szName)
{
	auto it = m_SubSystemMrg.find(szName);
	if (m_SubSystemMrg.end() == it)
	{
		return true;
	}

	it.get_data()->OnShut();
	m_SubSystemMrg.eraser(szName);
	return true;
}

IEntity* Context::GetSubsystem(const char* szName) const
{
	auto it = m_SubSystemMrg.find(szName);
	if (m_SubSystemMrg.end() == it)
	{
		return nullptr;
	}

	return it.get_data().get();
}

IEntityPtr Context::GetEntity(const PERSISTID& obj)
{
	auto it = m_EntityMrg.find(obj);
	if (m_EntityMrg.end() == it)
	{
		return nullptr;
	}

	return it->second;
}

void Context::AddEntity(const PERSISTID& obj, IEntityPtr pEntity)
{
	m_EntityMrg[obj] = pEntity;
}

bool Context::RemoveEntity(const PERSISTID& obj)
{
	auto it = m_EntityMrg.find(obj);
	if (m_EntityMrg.end() == it)
	{
		return true;
	}

	m_EntityMrg.erase(obj);
	return true;
}

