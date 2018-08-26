#include "../Core/CHelper.h"
#include "../../Game/Util/UtilTool.h"

#include <boost/any.hpp>
extern ICore *g_pCore;
bool CHelper::FindCustom(const IEntity* obj, const char* szName)
{
	if (nullptr == obj)
	{
		return false;
	}

	if (nullptr == obj->GetCustom())
	{
		return false;
	}
	if (obj->GetCustom()->FindData(szName))
	{
		return true;
	}

	return false;
}

bool CHelper::RemoveCustom(const IEntity* obj, const char* szName)
{
	if (nullptr == obj)
	{
		return false;
	}

	if (nullptr == obj->GetCustom())
	{
		return false;
	}

	return obj->GetCustom()->RemoveData(szName);
}

bool CHelper::SetCustomBool(const IEntity* obj, const char* szName, bool value)
{
	if (nullptr == obj)
	{
		return false;
	}
	if (nullptr == obj->GetCustom())
	{
		return false;
	}

	if (FindCustom(obj, szName))
	{
		return obj->GetCustom()->SetDataBool(szName, value);
	} 
	else
	{
		return obj->GetCustom()->AddDataBool(szName, value);
	}
	return true;
}

bool CHelper::SetCustomInt(const IEntity* obj, const char* szName, int value)
{
	if (nullptr == obj)
	{
		return false;
	}
	if (nullptr == obj->GetCustom())
	{
		return false;
	}

	if (FindCustom(obj, szName))
	{
		return obj->GetCustom()->SetDataInt(szName, value);
	}
	else
	{
		return obj->GetCustom()->AddDataInt(szName, value);
	}
	return true;
}

bool CHelper::SetCustomInt64(const IEntity* obj, const char* szName, int64_t value)
{
	if (nullptr == obj)
	{
		return false;
	}
	if (nullptr == obj->GetCustom())
	{
		return false;
	}

	if (FindCustom(obj, szName))
	{
		return obj->GetCustom()->SetDataInt64(szName, value);
	}
	else
	{
		return obj->GetCustom()->AddDataInt64(szName, value);
	}
	return true;
}

bool CHelper::SetCustomFloat(const IEntity* obj, const char* szName, float value)
{
	if (nullptr == obj)
	{
		return false;
	}
	if (nullptr == obj->GetCustom())
	{
		return false;
	}

	if (FindCustom(obj, szName))
	{
		return obj->GetCustom()->SetDataFloat(szName, value);
	}
	else
	{
		return obj->GetCustom()->AddDataFloat(szName, value);
	}
	return true;
}

bool CHelper::SetCustomDouble(const IEntity* obj, const char* szName, double value)
{
	if (nullptr == obj)
	{
		return false;
	}
	if (nullptr == obj->GetCustom())
	{
		return false;
	}

	if (FindCustom(obj, szName))
	{
		return obj->GetCustom()->SetDataDouble(szName, value);
	}
	else
	{
		return obj->GetCustom()->AddDataDouble(szName, value);
	}
	return true;
}

bool CHelper::SetCustomString(const IEntity* obj, const char* szName, const char* value)
{
	if (nullptr == obj)
	{
		return false;
	}
	if (nullptr == obj->GetCustom())
	{
		return false;
	}

	if (FindCustom(obj, szName))
	{
		return obj->GetCustom()->SetDataString(szName, value);
	}
	else
	{
		return obj->GetCustom()->AddDataString(szName, value);
	}
	return true;
}

bool CHelper::SetCustomWideStr(const IEntity* obj, const char* szName, const wchar_t* value)
{
	if (nullptr == obj)
	{
		return false;
	}
	if (nullptr == obj->GetCustom())
	{
		return false;
	}
	
	if (FindCustom(obj, szName))
	{
		return obj->GetCustom()->SetDataWideStr(szName, value);
	}
	else
	{
		return obj->GetCustom()->AddDataWideStr(szName, value);
	}
	return true;
}

bool CHelper::SetCustomObject(const IEntity* obj, const char* szName, const PERSISTID& value)
{
	if (nullptr == obj)
	{
		return false;
	}
	if (nullptr == obj->GetCustom())
	{
		return false;
	}

	if (FindCustom(obj, szName))
	{
		return obj->GetCustom()->SetDataObject(szName, value);
	}
	else
	{
		return obj->GetCustom()->AddDataObject(szName, value);
	}
	return true;
}

bool CHelper::QueryCustombool(const IEntity* obj, const char* szName)
{
	if (nullptr != obj && nullptr != obj->GetCustom())
	{
		if (FindCustom(obj, szName))
		{
			return obj->GetCustom()->GetDatabool(szName);
		}
	}

	return false;
}

int CHelper::QueryCustomInt(const IEntity* obj, const char* szName)
{
	if (nullptr != obj && nullptr != obj->GetCustom())
	{
		if (FindCustom(obj, szName))
		{
			return obj->GetCustom()->GetDataInt(szName);
		}
	}

	return 0;
}

int64_t CHelper::QueryCustomInt64(const IEntity* obj, const char* szName)
{
	if (nullptr != obj && nullptr != obj->GetCustom())
	{
		if (FindCustom(obj, szName))
		{
			return obj->GetCustom()->GetDataInt64(szName);
		}
	}

	return 0L;
}

PERSISTID CHelper::QueryCustomObject(const IEntity* obj, const char* szName)
{
	if (nullptr != obj && nullptr != obj->GetCustom())
	{
		if (FindCustom(obj, szName))
		{
			return obj->GetCustom()->GetDataObject(szName);
		}
	}

	return PERSISTID();
}

double CHelper::QueryCustomDouble(const IEntity* obj, const char* szName)
{
	if (nullptr != obj && nullptr != obj->GetCustom())
	{
		if (FindCustom(obj, szName))
		{
			return obj->GetCustom()->GetDataDouble(szName);
		}
	}

	return 0.0f;
}

float CHelper::QueryCustomFloat(const IEntity* obj, const char* szName)
{
	if (nullptr != obj && nullptr != obj->GetCustom())
	{
		if (FindCustom(obj, szName))
		{
			return obj->GetCustom()->GetDataFloat(szName);
		}
	}

	return 0.0f;
}

const char* CHelper::QueryCustomString(const IEntity* obj, const char* szName)
{
	if (nullptr != obj && nullptr != obj->GetCustom())
	{
		if (FindCustom(obj, szName))
		{
			return obj->GetCustom()->GetDataString(szName);
		}
	}

	return "";
}

const wchar_t* CHelper::QueryCustomWideStr(const IEntity* obj, const char* szName)
{
	if (nullptr != obj && nullptr != obj->GetCustom())
	{
		if (FindCustom(obj, szName))
		{
			return obj->GetCustom()->GetDataWideStr(szName);
		}
	}

	return L"";
}

bool CHelper::FindGlobal(const char* szName)
{
	return g_pCore->FindGlobalValue(szName);
}

bool CHelper::RemoveGlobal(const char* szName)
{
	return g_pCore->RemoveGlobalValue(szName);
}

bool CHelper::SetGlobalBool(const char* szName, bool value)
{
	return g_pCore->SetGlobalValue(szName, boost::any(value));
}

bool CHelper::SetGlobalInt(const char* szName, int value)
{
	return g_pCore->SetGlobalValue(szName, boost::any(value));
}

bool CHelper::SetGlobalInt64(const char* szName, int64_t value)
{
	return g_pCore->SetGlobalValue(szName, boost::any(value));
}

bool CHelper::SetGlobalFloat(const char* szName, float value)
{
	return g_pCore->SetGlobalValue(szName, boost::any(value));
}

bool CHelper::SetGlobalDouble(const char* szName, double value)
{
	return g_pCore->SetGlobalValue(szName, boost::any(value));
}

bool CHelper::SetGlobalString(const char* szName, const char* value)
{
	return g_pCore->SetGlobalValue(szName, boost::any(value));
}

bool CHelper::SetGlobalWideStr(const char* szName, const wchar_t* value)
{
	return g_pCore->SetGlobalValue(szName, boost::any(value));
}

bool CHelper::SetGlobalObject(const char* szName, const PERSISTID& value)
{
	return g_pCore->SetGlobalValue(szName, boost::any(value));
}

bool CHelper::QueryGlobalbool(const char* szName)
{
	if (!g_pCore->FindGlobalValue(szName))
	{
		return false;
	}

	auto var = g_pCore->GetGlobalValue(szName);
	return *(boost::any_cast<bool>(&var));
}

int CHelper::QueryCGlobalInt(const char* szName)
{
	if (!g_pCore->FindGlobalValue(szName))
	{
		return 0;
	}

	auto var = g_pCore->GetGlobalValue(szName);
	return *(boost::any_cast<int>(&var));
}

int64_t CHelper::QueryGlobalInt64(const char* szName)
{
	if (!g_pCore->FindGlobalValue(szName))
	{
		return 0;
	}

	auto var = g_pCore->GetGlobalValue(szName);
	return *(boost::any_cast<int64_t>(&var));
}

PERSISTID CHelper::QueryGlobalObject(const char* szName)
{
	if (!g_pCore->FindGlobalValue(szName))
	{
		return PERSISTID();
	}

	auto var = g_pCore->GetGlobalValue(szName);
	return *(boost::any_cast<PERSISTID>(&var));
}

double CHelper::QueryGlobalDoublet(const char* szName)
{
	if (!g_pCore->FindGlobalValue(szName))
	{
		return 0.0;
	}

	auto var = g_pCore->GetGlobalValue(szName);
	return *(boost::any_cast<double>(&var));
}

float CHelper::QueryGlobalFloat(const char* szName)
{
	if (!g_pCore->FindGlobalValue(szName))
	{
		return 0.0f;
	}

	auto var = g_pCore->GetGlobalValue(szName);
	return *(boost::any_cast<float>(&var));
}

const char* CHelper::QueryGlobalString(const char* szName)
{
	if (!g_pCore->FindGlobalValue(szName))
	{
		return "";
	}

	auto var = g_pCore->GetGlobalValue(szName);
	return boost::any_cast<const char>(&var);
}

const wchar_t* CHelper::QueryGlobalWideStr(const char* szName)
{
	if (!g_pCore->FindGlobalValue(szName))
	{
		return L"";
	}

	auto var = g_pCore->GetGlobalValue(szName);
	return boost::any_cast<const wchar_t>(&var);
}

std::string CHelper::GetRecource()
{
	return g_pCore->GetResource();
}

void CHelper::TraceLog(int nType, char* szMsg, ...)
{
	//g_pCore->Trace_Log(nType, szMsg);
}

IEntity* CHelper::GetEntity(const char *szName)
{
	return g_pCore->GetEntity(szName);
}

IEntity* CHelper::CreateEntityArgs(const char *szName)
{
	return g_pCore->CreateEntityArgs(szName);
}

void CHelper::RemoveEntity(const char* szName)
{
	IEntity* entity = g_pCore->GetEntity(szName);
	if (nullptr != entity)
	{
		g_pCore->RemoveEntity(entity->GetID());
	}
}

IEntity* CHelper::CreateEntity(const char *szName)
{
	IEntity* entity = g_pCore->CreateEntity(szName);
	if (nullptr != entity)
	{
		g_pCore->SetGlobalValue(szName, entity);
		return entity;
	}

	return nullptr;
}


