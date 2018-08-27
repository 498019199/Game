#ifndef _PERSISTID_
#define _PERSISTID_
#pragma once
#include "../Container/var_type.h"

class PERSISTID
{
public:
	PERSISTID(uint64_t obj = 0) :m_Data(obj) {}

	PERSISTID(int nIdent, int nSerial) 
		:m_nIdent(nIdent), m_nSerial(nSerial) {}

	PERSISTID(const PERSISTID& rhs)
	{
		m_Data = rhs.m_Data;
	}

	PERSISTID operator=(const PERSISTID& rhs)
	{
		m_Data = rhs.m_Data;
		
		return m_Data;
	}

	bool IsNull()
	{
		return 0 == m_Data;
	}

	bool operator==(const PERSISTID& rhs)
	{
		return this->GetData() == rhs.GetData();
	}

    int GetSerial() const
    {
        return m_nSerial;
    }

    int GetIdent() const
    {
        return m_nIdent;
    }

	int64_t GetData() const
	{
		return m_Data;
	}

	bool operator<(const PERSISTID& that) const
	{
		return this->GetData() < that.GetData();
	}
private:
	union
	{
		uint64_t m_Data;
		struct
		{
			int m_nSerial;
			int m_nIdent;
		};
	};
};

inline bool operator==(const PERSISTID& lhs, const PERSISTID& rhs)
{
	return lhs.GetData() == rhs.GetData();
}
#endif//_PERSISTID_