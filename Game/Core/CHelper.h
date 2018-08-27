#ifndef _HELPER_H_
#define _HELPER_H_
#pragma once
#include "../Core/Entity/Entity.h"
#include "../Container/i_varlist.h"
#include "../Core/Context.h"
#include <string>

enum LogType
{
	LOG_LOGIC_NONE = 0,
	LOG_LOGIC_FATAL = 1,
	LOG_LOGIC_ERROR = 2,
	LOG_LOGIC_WARING = 3,
	LOG_LOGIC_INFO = 4,
	LOG_LOGIC_DEBUG = 5,
	LOG_LOGIC_TRACE = 6,
	LOG_LOGIC_ALL = 7,
};

class CHelper
{
public:
    //设置临时值
	static bool FindCustom(const IEntity* obj, const char* szName);
	static bool RemoveCustom(const IEntity* obj, const char* szName);
	static bool SetCustomBool(const IEntity* obj, const char* szName, bool value);
	static bool SetCustomInt(const IEntity* obj, const char* szName, int value);
	static bool SetCustomInt64(const IEntity* obj, const char* szName, int64_t value);
	static bool SetCustomFloat(const IEntity* obj, const char* szName, float value);
	static bool SetCustomDouble(const IEntity* obj, const char* szName, double value);
	static bool SetCustomString(const IEntity* obj, const char* szName, const char* value);
	static bool SetCustomWideStr(const IEntity* obj, const char* szName, const wchar_t* value);
	static bool SetCustomObject(const IEntity* obj, const char* szName, const PERSISTID& value);
    //获取临时值
	static bool QueryCustombool(const IEntity* obj, const char* szName);
	static int QueryCustomInt(const IEntity* obj, const char* szName);
	static int64_t QueryCustomInt64(const IEntity* obj, const char* szName);
	static PERSISTID QueryCustomObject(const IEntity* obj, const char* szName);
	static double QueryCustomDouble(const IEntity* obj, const char* szName);
	static float QueryCustomFloat(const IEntity* obj, const char* szName);
	static const char* QueryCustomString(const IEntity* obj, const char* szName);
	static const wchar_t* QueryCustomWideStr(const IEntity* obj, const char* szName);
    //设置全局临时值
	static bool FindGlobal(const char* szName);
	static bool RemoveGlobal(const char* szName);
	static bool SetGlobalBool(const char* szName, bool value);
	static bool SetGlobalInt(const char* szName, int value);
	static bool SetGlobalInt64(const char* szName, int64_t value);
	static bool SetGlobalFloat(const char* szName, float value);
	static bool SetGlobalDouble(const char* szName, double value);
	static bool SetGlobalString(const char* szName, const char* value);
	static bool SetGlobalWideStr(const char* szName, const wchar_t* value);
	static bool SetGlobalObject(const char* szName, const PERSISTID& value);
    //获取全局临时值
	static bool QueryGlobalbool(const char* szName);
	static int QueryCGlobalInt(const char* szName);
	static int64_t QueryGlobalInt64(const char* szName);
	static PERSISTID QueryGlobalObject(const char* szName);
	static double QueryGlobalDoublet(const char* szName);
    static float QueryGlobalFloat(const char* szName);
	static const String QueryGlobalString(const char* szName);
	static const WString  QueryGlobalWideStr(const char* szName);

    //获取资源路径
    static std::string GetRecource();

	static void TraceLog(int nType, char* szMsg, ...);
};

#endif//_HELPER_H_