#include "cvar_list.h"
#include "../Util/UtilTool.h"

Variable::Variable()
{
}

Variable::~Variable()
{
}

Variable& Variable::operator=(const PERSISTID& value)
{
	UNREACHABLE_MSG("Can't be called");
}

Variable& Variable::operator=(const WString& value)
{
	UNREACHABLE_MSG("Can't be called");
}

Variable& Variable::operator=(const String& value)
{
	UNREACHABLE_MSG("Can't be called");
}

Variable& Variable::operator=(const double& value)
{
	UNREACHABLE_MSG("Can't be called");
}

Variable& Variable::operator=(const bool& value)
{
	UNREACHABLE_MSG("Can't be called");
}

Variable& Variable::operator=(const int& value)
{
	UNREACHABLE_MSG("Can't be called");
}

Variable& Variable::operator=(const int64_t& value)
{
	UNREACHABLE_MSG("Can't be called");
}

Variable& Variable::operator=(const float& value)
{
	UNREACHABLE_MSG("Can't be called");
}

Variable& Variable::operator=(const float2& value)
{
	UNREACHABLE_MSG("Can't be called");
}

Variable& Variable::operator=(const float3& value)
{
	UNREACHABLE_MSG("Can't be called");
}

Variable& Variable::operator=(const float4& value)
{
	UNREACHABLE_MSG("Can't be called");
}

Variable& Variable::operator=(const float4x4& value)
{
	UNREACHABLE_MSG("Can't be called");
}

int Variable::GetType() const
{
	return 0;
}

void Variable::Value(bool& val) const
{
	UNREACHABLE_MSG("Can't be called");
}

void Variable::Value(int& val) const
{
	UNREACHABLE_MSG("Can't be called");
}

void Variable::Value(int64_t& val) const
{
	UNREACHABLE_MSG("Can't be called");
}

void Variable::Value(float& val) const
{
	UNREACHABLE_MSG("Can't be called");
}

void Variable::Value(double& val) const
{
	UNREACHABLE_MSG("Can't be called");
}

void Variable::Value(String& val) const
{
	UNREACHABLE_MSG("Can't be called");
}

void Variable::Value(WString& val) const
{
	UNREACHABLE_MSG("Can't be called");
}

void Variable::Value(PERSISTID& val) const
{
	UNREACHABLE_MSG("Can't be called");
}

void Variable::Value(float2& val) const
{
	UNREACHABLE_MSG("Can't be called");
}

void Variable::Value(float3& val) const
{
	UNREACHABLE_MSG("Can't be called");
}

void Variable::Value(float4& val) const
{
	UNREACHABLE_MSG("Can't be called");
}

void Variable::Value(float4x4& val) const
{
	UNREACHABLE_MSG("Can't be called");
}

