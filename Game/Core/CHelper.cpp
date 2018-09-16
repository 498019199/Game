#include "../Core/CHelper.h"
#include "../../Game/Util/UtilTool.h"

#include <boost/any.hpp>
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
	return Context::Instance()->FindGlobalValue(szName);
}

bool CHelper::RemoveGlobal(const char* szName)
{
	return Context::Instance()->RemoveGlobalValue(szName);
}

bool CHelper::SetGlobalBool(const char* szName, bool value)
{
	auto val = MakeSharedPtr<VariableBool>();
	val->SetType(CType_bool);
	*val = value;
	return Context::Instance()->SetGlobalValue(szName, val);
}

bool CHelper::SetGlobalInt(const char* szName, int value)
{
	auto val = MakeSharedPtr<VariableInt>();
	*val = value;
	val->SetType(CType_int);
	return Context::Instance()->SetGlobalValue(szName, val);
}

bool CHelper::SetGlobalInt64(const char* szName, int64_t value)
{
	auto val = MakeSharedPtr<VariableInt64>();
	*val = value;
	val->SetType(CType_int64);
	return Context::Instance()->SetGlobalValue(szName, val);
}

bool CHelper::SetGlobalFloat(const char* szName, float value)
{
	auto val = MakeSharedPtr<VariableFloat>();
	*val = value;
	val->SetType(CType_float);
	return Context::Instance()->SetGlobalValue(szName, val);
}

bool CHelper::SetGlobalDouble(const char* szName, double value)
{
	auto val = MakeSharedPtr<VariableDouble>();
	*val = value;
	val->SetType(CType_double);
	return Context::Instance()->SetGlobalValue(szName, val);
}

bool CHelper::SetGlobalString(const char* szName, const char* value)
{
	auto val = MakeSharedPtr<VariableString>();
	*val = String(value);
	val->SetType(CType_string);
	return Context::Instance()->SetGlobalValue(szName, val);
}

bool CHelper::SetGlobalWideStr(const char* szName, const wchar_t* value)
{
	auto val = MakeSharedPtr<VariableWString>();
	*val = WString(value);
	val->SetType(CType_widestring);
	return Context::Instance()->SetGlobalValue(szName, val);
}

bool CHelper::SetGlobalObject(const char* szName, const PERSISTID& value)
{
	auto val = MakeSharedPtr<VariablePERSISTID>();
	*val = value;
	val->SetType(CType_object);
	return Context::Instance()->SetGlobalValue(szName, val);
}

bool CHelper::QueryGlobalbool(const char* szName)
{
	if (!Context::Instance()->FindGlobalValue(szName))
	{
		return false;
	}

	bool tmp;
	auto var = Context::Instance()->GetGlobalValue(szName);
	var->Value(tmp);
	return tmp;
}

int CHelper::QueryCGlobalInt(const char* szName)
{
	if (!Context::Instance()->FindGlobalValue(szName))
	{
		return 0;
	}

	int tmp;
	auto var = Context::Instance()->GetGlobalValue(szName);
	var->Value(tmp);
	return tmp;
}

int64_t CHelper::QueryGlobalInt64(const char* szName)
{
	if (!Context::Instance()->FindGlobalValue(szName))
	{
		return 0;
	}

	int64_t tmp;
	auto var = Context::Instance()->GetGlobalValue(szName);
	var->Value(tmp);
	return tmp;
}

PERSISTID CHelper::QueryGlobalObject(const char* szName)
{
	if (!Context::Instance()->FindGlobalValue(szName))
	{
		return PERSISTID();
	}

	PERSISTID tmp;
	auto var = Context::Instance()->GetGlobalValue(szName);
	var->Value(tmp);
	return tmp;
}

double CHelper::QueryGlobalDoublet(const char* szName)
{
	if (!Context::Instance()->FindGlobalValue(szName))
	{
		return 0.0;
	}

	double tmp;
	auto var = Context::Instance()->GetGlobalValue(szName);
	var->Value(tmp);
	return tmp;
}

float CHelper::QueryGlobalFloat(const char* szName)
{
	if (!Context::Instance()->FindGlobalValue(szName))
	{
		return 0.0f;
	}

	float tmp;
	auto var = Context::Instance()->GetGlobalValue(szName);
	var->Value(tmp);
	return tmp;
}

const String CHelper::QueryGlobalString(const char* szName)
{
	if (!Context::Instance()->FindGlobalValue(szName))
	{
		return "";
	}

	String tmp;
	auto var = Context::Instance()->GetGlobalValue(szName);
	var->Value(tmp);
	return tmp;
}

const WString CHelper::QueryGlobalWideStr(const char* szName)
{
	if (!Context::Instance()->FindGlobalValue(szName))
	{
		return L"";
	}

	WString tmp;
	auto var = Context::Instance()->GetGlobalValue(szName);
	var->Value(tmp);
	return tmp;
}

void CHelper::TraceLog(int nType, char* szMsg, ...)
{
	//Context::Instance()->Trace_Log(nType, szMsg);
}



