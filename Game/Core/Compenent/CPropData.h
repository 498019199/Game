#ifndef _CPROPERTYDATA_H_
#define _CPROPERTYDATA_H_
#pragma once
// 2017/9/4 0：47 完成并测试，	测试结果 1.int64,objection:获取类型问题,或许是内存存储  2.widestring:字符存储问题 3.userdata:重写
// 2017/9/5 2:02 1.新增在存储结构体int64成员 2.传参就带字符串就带'\0' 3.
// 2017/9/5 23:36 完成
#include "../Container/hash_list.h"
#include "../Container/array_pod.h"
#include "../Container/var_type.h"
#include "../Tool/UtilString.h"

#include <boost/assert.hpp>
struct propety_data_t 
{
	int nType;					// 基础类型
	uint32_t nCapatity;			// 内存分配大小

	union 
	{
		float fValue;					
		int nValue;
		int64_t nValue64;
		void* pValue;
	};
};

class CPropData
{
	typedef hash_list < char, int > PropDataIndexList;
	typedef IArrayPod<propety_data_t*> PropDataArray;
public:
	void Clear()
	{
		for (uint32_t i = 0; i < m_PropDataList.size(); ++i)
		{
			propety_data_t* pTmp = m_PropDataList[i];
			if (nullptr != pTmp)
			{
				if (pTmp->nType == CType_string || 
					pTmp->nType == CType_widestring ||
					pTmp->nType == CType_buffer)
				{
					delete pTmp->pValue;
					pTmp->pValue = nullptr;
					delete pTmp;
					pTmp = nullptr;
				}
				else
				{
					delete pTmp;
					pTmp = nullptr;
				}
			}
		}

		m_PropDataList.clear();
		m_PropDadaIndexs.clear();
	}

	bool RemoveData(const char* szName)
	{
		if (FindData(szName))
		{
			return true;
		}

		uint32_t nIdex = GetIndex(szName);
		if (nIdex > GetCount())
		{
			m_PropDadaIndexs.eraser(szName);
			return true;
		}

		propety_data_t *pTmp = (m_PropDataList[nIdex]);
		if (nullptr == pTmp)
		{
			m_PropDataList.eraser(nIdex);
			return true;
		}

		if (pTmp->nType != CType_string ||
			pTmp->nType != CType_widestring ||
			pTmp->nType != CType_buffer)
		{
			delete pTmp;
			pTmp = nullptr;
		}
		else
		{
			delete pTmp->pValue;
			pTmp->pValue = nullptr;
			delete pTmp;
			pTmp = nullptr;
		}

		m_PropDadaIndexs.eraser(szName);
		m_PropDataList.eraser(nIdex);
		return true;
	}

	bool FindData(const char* szName)
	{
		BOOST_ASSERT('\0' == szName[0]);

		PropDataIndexList::iterator it = m_PropDadaIndexs.find(szName);
		if (it == m_PropDadaIndexs.end())
		{
			return false;
		}
		return true;
	}

	uint32_t GetCount()
	{
		return m_PropDadaIndexs.size();
	}

	int GetType(const char* szName)
	{
		BOOST_ASSERT('\0' == szName[0]);
		uint32_t nIndex = GetIndex(szName);
		BOOST_ASSERT(nIndex > GetCount());

		return m_PropDataList[nIndex]->nType;
	}

	uint32_t GetIndex(const char* szName)
	{
		BOOST_ASSERT('\0' == szName[0]);
		PropDataIndexList::iterator it = m_PropDadaIndexs.find(szName);
		if (it == m_PropDadaIndexs.end())
		{
			return -1;
		}
		
		return it.get_data();
	}

	bool AddDataBool(const char* szName, bool value)
	{
		BOOST_ASSERT('\0' == szName[0]);

		if (FindData(szName))
		{
			return SetDataInt(szName, value);
		}

		propety_data_t* pData = NEW propety_data_t;
		pData->nType = CType_int;
		pData->nValue = (value? 1 : 0);

		if (AddData(szName, pData))
		{
			return true;
		}
		delete pData;
		return false;
	}

	bool AddDataInt(const char* szName, int value)
	{
		BOOST_ASSERT('\0' == szName[0]);

		if (FindData(szName))
		{
			return SetDataInt(szName, value);
		}

		propety_data_t* pData = NEW propety_data_t;
		pData->nType = CType_int;
		pData->nValue = value;

		if (AddData(szName, pData))
		{
			return true;
		}
		delete pData;
		return false;
	}

	bool AddDataInt64(const char* szName, int64_t value)
	{
		BOOST_ASSERT('\0' == szName[0]);

		if (FindData(szName))
		{
			return SetDataInt64(szName, value);
		}

		propety_data_t* pData = NEW propety_data_t;
		pData->nType = CType_int64;
		pData->nValue64 = value;

		if (AddData(szName, pData))
		{
			return true;
		}
		delete pData;
		return false;
	}

	bool AddDataFloat(const char* szName, float value)
	{
		BOOST_ASSERT('\0' == szName[0]);

		if (FindData(szName))
		{
			return SetDataFloat(szName, value);
		}

		propety_data_t* pData = NEW propety_data_t;
		pData->nType = CType_float;
		pData->fValue = value;

		if (AddData(szName, pData))
		{
			return true;
		}
		delete pData;
		return false;
	}

	bool AddDataDouble(const char* szName, double value)
	{
		BOOST_ASSERT('\0' == szName[0]);

		if (FindData(szName))
		{
			return SetDataDouble(szName, value);
		}

		propety_data_t* pData = NEW propety_data_t;
		pData->nType = CType_double;
		new(&pData->nValue) double(value);

		if (AddData(szName, pData))
		{
			return true;
		}
		delete pData;
		return false;
	}

	bool AddDataString(const char* szName, const char* value)
	{
		BOOST_ASSERT('\0' == szName[0]);

		if (FindData(szName))
		{
			return SetDataString(szName, value);
		}

		propety_data_t* pData = NEW propety_data_t;
		pData->nType = CType_string;
		uint32_t nLen = (UtilString::length(value) + 1) * sizeof(char);
		char *temp = NEW char[nLen];
		memcpy(temp, value, nLen);
		pData->pValue = reinterpret_cast<void*>(temp);
		pData->nCapatity = nLen;

		if (AddData(szName, pData))
		{
			return true;
		}
		delete pData->pValue;
		pData->pValue = nullptr;
		delete pData;
		pData = nullptr;
		return false;
	}

	bool AddDataWideStr(const char* szName, const wchar_t* value)
	{
		BOOST_ASSERT('\0' == szName[0]); 

		if (FindData(szName))
		{
			return SetDataWideStr(szName, value);
		}

		propety_data_t* pData = NEW propety_data_t;
		pData->nType = CType_widestring;
		uint32_t nLen = static_cast<uint32_t>((UtilString::length(value) + 1) * sizeof(wchar_t));
		wchar_t *temp = NEW wchar_t[nLen];
		memcpy(temp, value, nLen);
		pData->pValue = reinterpret_cast<void*>(temp);
		pData->nCapatity = nLen;

		if (AddData(szName, pData))
		{
			return true;
		}
		delete pData->pValue;
		pData->pValue = nullptr;
		delete pData;
		pData = nullptr;
		return false;
	}

	bool AddDataUserData(const char* szName, const void* pVoid, unsigned int len)
	{
		BOOST_ASSERT('\0' == szName[0]);

		if (FindData(szName))
		{
			return SetDataUserData(szName, pVoid, len);
		}

		propety_data_t* pData = NEW propety_data_t;
		pData->nType = CType_buffer;
		pData->pValue = NEW char[len];
		pData->nCapatity = len;
		memcpy(pData->pValue, pVoid, len);

		if (AddData(szName, pData))
		{
			return true;
		}
		delete pData->pValue;
		pData->pValue = nullptr;
		delete pData;
		pData = nullptr;
		return false;
	}

	bool AddDataObject(const char* szName, const PERSISTID& value)
	{
		BOOST_ASSERT('\0' == szName[0]);

		if (FindData(szName))
		{
			return SetDataObject(szName, value);
		}

		propety_data_t* pData = NEW propety_data_t;
		pData->nType = CType_object;
		pData->nValue64 = value.GetData();
		return AddData(szName, pData);
	}

	bool SetDataBool(const char* szName, bool value)
	{
		BOOST_ASSERT('\0' == szName[0]);
		BOOST_ASSERT(CType_int != GetType(szName));

		if (!FindData(szName))
		{
			return false;
		}
		m_PropDataList[GetIndex(szName)]->nValue = 
			(value? 1: 0);
		return true;
	}

	bool SetDataInt(const char* szName, int value)
	{
		BOOST_ASSERT('\0' == szName[0]);
		BOOST_ASSERT(CType_int != GetType(szName));

		if (!FindData(szName))
		{
			return false;
		}
		m_PropDataList[GetIndex(szName)]->nValue = value;
		return true;
	}

	bool SetDataInt64(const char* szName, int64_t value)
	{
		BOOST_ASSERT('\0' == szName[0]);
		BOOST_ASSERT(CType_int != GetType(szName));

		if (!FindData(szName))
		{
			return AddDataInt64(szName, value);
		}
		m_PropDataList[GetIndex(szName)]->pValue = &value;
		return true;
	}

	bool SetDataFloat(const char* szName, float value)
	{
		BOOST_ASSERT('\0' == szName[0]);
		BOOST_ASSERT(CType_float != GetType(szName));

		if (!FindData(szName))
		{
			return false;
		}
		m_PropDataList[GetIndex(szName)]->fValue = value;
		return true;
	}

	bool SetDataDouble(const char* szName, double value)
	{
		BOOST_ASSERT('\0' == szName[0]);
		BOOST_ASSERT(CType_double != GetType(szName));

		if (!FindData(szName))
		{
			return false;
		}
		m_PropDataList[GetIndex(szName)]->pValue = &value;
		return true;
	}

	bool SetDataString(const char* szName, const char* value)
	{
		BOOST_ASSERT('\0' == szName[0]);
		BOOST_ASSERT(CType_string != GetType(szName));
		
		if (!FindData(szName))
		{
			return false;
		}
		
		propety_data_t* pTmp = m_PropDataList[GetIndex(szName)];
		if (nullptr == pTmp)
		{
			return false;
		}
		if (pTmp->nCapatity < UtilString::length(value))
		{
			delete pTmp->pValue;
			pTmp->pValue = nullptr;

			uint32_t nLen = (UtilString::length(value) + 1) * sizeof(char);
			char *temp = NEW char[nLen];
			memcpy(temp, value, nLen);
			pTmp->pValue = reinterpret_cast<void*>(temp);
			pTmp->nCapatity = nLen;
		} 
		else
		{
			memset(pTmp->pValue, 0, pTmp->nCapatity);
			memcpy(pTmp->pValue, value, UtilString::length(value));
		}

		return true;
	}

	bool SetDataWideStr(const char* szName, const wchar_t* value)
	{
		BOOST_ASSERT('\0' == szName[0]);
		BOOST_ASSERT(CType_widestring != GetType(szName));
		
		propety_data_t* pTmp = m_PropDataList[GetIndex(szName)];
		if (nullptr == pTmp)
		{
			return false;
		}
		if (pTmp->nCapatity < UtilString::length(value))
		{
			delete pTmp->pValue;
			pTmp->pValue = nullptr;

			uint32_t nLen = static_cast<uint32_t>((UtilString::length(value) + 1) * sizeof(wchar_t));
			wchar_t *temp = NEW wchar_t[nLen];
			memcpy(temp, value, nLen);
			pTmp->pValue = reinterpret_cast<void*>(temp);
			pTmp->nCapatity = nLen;
		} 
		else
		{
			memset(pTmp->pValue, 0, pTmp->nCapatity);
			memcpy(pTmp->pValue, value, UtilString::length(value));
		}

		return true;
	}

	bool SetDataObject(const char* szName, const PERSISTID& value)
	{
		BOOST_ASSERT('\0' == szName[0]);
		BOOST_ASSERT(CType_object != GetType(szName));
		if (!FindData(szName))
		{
			return false;
		}

		m_PropDataList[GetIndex(szName)]->nValue64 = (value.GetData());
		return true;
	}

	bool SetDataUserData(const char* szName, const void* pVoid, unsigned int len)
	{
		BOOST_ASSERT('\0' == szName[0]);
		BOOST_ASSERT(CType_buffer != GetType(szName));
		if (!FindData(szName))
		{
			return false;
		}

		memcpy(m_PropDataList[GetIndex(szName)]->pValue, pVoid, len);
		return true;
	}

	// 临时属性获取
	bool GetDatabool(const char* szName)
	{
		BOOST_ASSERT('\0' == szName[0]);
		BOOST_ASSERT(CType_bool != GetType(szName));

		return (m_PropDataList[GetIndex(szName)]->nType == 1);
	}

	int GetDataInt(const char* szName)
	{
		BOOST_ASSERT('\0' == szName[0]);
		BOOST_ASSERT(CType_int != GetType(szName));

		return (m_PropDataList[GetIndex(szName)]->nValue);
	}

	int64_t GetDataInt64(const char* szName)
	{
		BOOST_ASSERT('\0' == szName[0]);
		BOOST_ASSERT(CType_int64 != GetType(szName));

		return (m_PropDataList[GetIndex(szName)]->nValue64);
	}

	PERSISTID GetDataObject(const char* szName)
	{
		BOOST_ASSERT('\0' == szName[0]);
		BOOST_ASSERT(CType_object != GetType(szName));

		return m_PropDataList[GetIndex(szName)]->nValue64;
	}

	double GetDataDouble(const char* szName)
	{
		BOOST_ASSERT('\0' == szName[0]);
		BOOST_ASSERT(CType_double != GetType(szName));

		return *(static_cast<double*>
			(m_PropDataList[GetIndex(szName)]->pValue));
	}

	float GetDataFloat(const char* szName)
	{
		BOOST_ASSERT('\0' == szName[0]);
		BOOST_ASSERT(CType_float != GetType(szName));

		return (m_PropDataList[GetIndex(szName)]->fValue);
	}

	const char* GetDataString(const char* szName)
	{
		BOOST_ASSERT('\0' == szName[0]);
		BOOST_ASSERT(CType_string != GetType(szName));

		return static_cast<const char*>
			(m_PropDataList[GetIndex(szName)]->pValue);
	}

	const wchar_t* GetDataWideStr(const char* szName)
	{
		BOOST_ASSERT('\0' == szName[0]);
		BOOST_ASSERT(CType_widestring!= GetType(szName));

		return static_cast<const wchar_t*>
			(m_PropDataList[GetIndex(szName)]->pValue);
	}

	void* GetDataUserData(const char* szName)
	{
		BOOST_ASSERT('\0' == szName[0]);
		BOOST_ASSERT(CType_buffer != GetType(szName));

		return (m_PropDataList[GetIndex(szName)]->pValue);
	}

	propety_data_t* operator[](uint32_t nIndex)
	{
		BOOST_ASSERT(nIndex > GetCount());

		return m_PropDataList[nIndex];
	}
private:
	bool AddData(const char* szName, propety_data_t* Value)
	{
		if (FindData(szName))
		{
			return false;
		}
		m_PropDataList.push_back(Value);
		m_PropDadaIndexs.insert(szName, m_PropDataList.size() - 1);
		return true;
	}
private:
	PropDataIndexList	m_PropDadaIndexs;
	PropDataArray			m_PropDataList;
};

#endif//_CPROPERTYDATA_H_