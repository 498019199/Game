#include "../Tool/UtilString.h"
#include "../public/momery.h"
#include "../Util/UtilTool.h"
#include "../Tool/XmlFile.h"

#include <cstdio>
#include <string>
#include <utility>
#include <fstream>
#include <iostream>
#include <boost/assert.hpp>

const char* strNodeRoot= "Object";
const char* strNodePropery = "Propety";

CXmlFile::CXmlFile(const char* file)
	:m_strFileName(file), m_pXmlValue(nullptr)
{}

CXmlFile::~CXmlFile()
{
    ReleaseRes();
}

const char* CXmlFile::GetFileName() const
{
    return m_strFileName.c_str();
}

bool CXmlFile::FileFormLord(std::string err_LogManager)
{
	ReleaseRes();
	//打开文件
	std::string path = m_strFileName;
	FILE* fp = fopen(path.c_str(), "rb");
	if (nullptr == fp)
	{
		err_LogManager = "open file fail!";
		return false;
	}
	//分配内存
	fseek(fp, 0, SEEK_END);
	std::size_t length = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	m_pXmlValue = NEW char[length + 1];
	if (nullptr == m_pXmlValue)
	{
		err_LogManager = "memory new fail!";
		return false;
	}
	memset(m_pXmlValue, 0, length + 1);
	//文件内存加载到内存
	fread(m_pXmlValue, length, 1, fp);
	fclose(fp);
	//把m_doc存储在vectior
	m_doc.clear();
	m_doc.parse<0>(m_pXmlValue);
	rapidxml::xml_node<>* pNodeRoot = nullptr;
	pNodeRoot = m_doc.first_node(strNodeRoot);
	if (nullptr == pNodeRoot)
	{
		err_LogManager = "root node is nullptr";
		delete[] m_pXmlValue;
		m_doc.clear();
		return false;
	}

	rapidxml::xml_node<>* pNodeProperty = nullptr;
	pNodeProperty = pNodeRoot->first_node();
	if (nullptr == pNodeProperty)
	{
		err_LogManager = "child node is nullptr";
		delete[] m_pXmlValue;
		m_doc.clear();
		return false;
	}
	while (pNodeProperty)
	{
        rapidxml::xml_attribute<>* attr = pNodeProperty->first_attribute();
        if (nullptr != attr)
        {
            std::size_t sHash = utilstring::hash_value(attr->value());
            section_t_xml *section = NEW section_t_xml(attr->value(), sHash);
			section->m_nStart = m_itemVec.size();

            rapidxml::xml_attribute<>* pXmlAttr = attr->next_attribute();
            for (; pXmlAttr; ++section->m_nCount)
            {
                std::size_t hash = utilstring::hash_value(pXmlAttr->name());
                item_t_xml *item = NEW item_t_xml(pXmlAttr->name(), hash, pXmlAttr->value());
                m_itemVec.push_back(item);
                pXmlAttr = pXmlAttr->next_attribute();
            }
            m_sectionMap.insert(std::make_pair(sHash
                , m_sectionVec.size()));
            m_sectionVec.push_back(section);
            pNodeProperty = pNodeProperty->next_sibling();
        }

	}
	return true;
}

bool CXmlFile::SetData(const char* sectionName, const char* item, const char* val)
{
    std::size_t item_index = GetItemIndex(sectionName, item);
    item_t_xml *pOldItem = m_itemVec[item_index];
    if (nullptr == pOldItem)
    {
        AddData(sectionName, item, val);
    }
    else
    {

        std::size_t hash = utilstring::hash_value(item);
        item_t_xml *pNewitem = NEW item_t_xml((char*)item, hash, (char*)val);
        m_itemVec[item_index] = pNewitem;
        delete pOldItem;
        pOldItem = nullptr;
    }

    return true;
}

void CXmlFile::Release()
{
	int iCount = (int)m_sectionVec.size();
	for (int i = 0; i < iCount; i++)
	{
		section_t_xml *pSection = m_sectionVec[i];
		if (nullptr != pSection)
		{
			delete pSection;
		}
	}
	m_sectionVec.clear();

	int jCount = (int)m_itemVec.size();
	for (int j = 0; j < jCount; j++)
	{
		item_t_xml *pItem = m_itemVec[j];
		if (nullptr != pItem)
		{
			delete pItem;
		}
	}
	m_itemVec.clear();
	m_sectionMap.clear();
	m_doc.clear();
	if (nullptr != m_pXmlValue)
	{
		delete[] m_pXmlValue;
	}
}

CXmlFile::section_t_xml* CXmlFile::GetSection(const char* section) const
{
	BOOST_ASSERT(nullptr == section);
	std::size_t SIZE = m_sectionVec.size();
    std::size_t hash = utilstring::hash_value(section);
	if (SIZE == m_sectionMap.size())
	{
		std::pair<sectionMapItem, sectionMapItem> ii = m_sectionMap.equal_range(hash);
		for (sectionMapItem i = ii.first; i != ii.second; ++i)
		{
			section_t_xml *pSection = m_sectionVec[i->second];
			if (nullptr != pSection &&
				pSection->m_nHash == hash &&
                0 == stricmp(pSection->m_strName, section))
			{
				return pSection;
			}
		}
	}
	else
	{
		for (std::size_t i = 0; i < m_sectionVec.size(); i++)
		{
			section_t_xml *pSection = m_sectionVec[i];
			if (nullptr != pSection &&
				pSection->m_nHash == hash &&
				0 == stricmp(pSection->m_strName, section))
			{
				return pSection;
			}
		}
	}
	return nullptr;
}

CXmlFile::item_t_xml* CXmlFile::GetItem(const char* section, const char* item) const
{
	BOOST_ASSERT(nullptr == section);
	BOOST_ASSERT(nullptr == item);
    const section_t_xml* pSection = GetSection(section);
    if (nullptr == pSection)
    {
        return nullptr;
    }
    std::size_t hash = utilstring::hash_value(item);
    std::size_t nCount = pSection->m_nStart + pSection->m_nCount;
    for (std::size_t i = pSection->m_nStart; i < nCount; i++)
    {
        item_t_xml* pItem = m_itemVec[i];
        if (nullptr != pItem
            && pItem->m_nHash == hash
            && 0 == strcmp(pItem->m_strName, item))
        {
            return pItem;
        }
    }

	return nullptr;
}

char* CXmlFile::GetData(const char* section, const char* item) const
{
	BOOST_ASSERT(nullptr == section);
	BOOST_ASSERT(nullptr == item);
    //std::size_t sec_index = 0;

	const item_t_xml* pItem = GetItem(section, item);
    if (nullptr == pItem)
    {
        return "";
    }
	return pItem->m_strValue;
}

CXmlFile::section_t_xml* CXmlFile::GetSectionByIndex(const std::size_t section_index) const
{
	if (section_index >= m_sectionVec.size())
	{
		return nullptr;
	}
	return m_sectionVec[section_index];
}

bool CXmlFile::AddSection(const char* sectionName)
{
    std::size_t sec_index = 0;
    if (FindSectionIndex(sectionName, sec_index))
    {
        return false;
    }

    std::size_t len = strlen(sectionName) + 1;
    char *new_name = NEW char[len];
    strncpy(new_name, sectionName, len);

    std::size_t hash = utilstring::hash_value(sectionName);
    section_t_xml* pSection = NEW section_t_xml(new_name, hash, true);
    pSection->m_nStart = m_itemVec.size();
    m_sectionVec.push_back(pSection);
    m_sectionMap.insert(std::make_pair(hash, pSection->m_nStart));
    return true;
}

bool CXmlFile::AddData(const char* sectionName, const char* item, const char* val)
{
    std::size_t sec_index = 0;
    if (!FindSectionIndex(sectionName, sec_index))
    {
        AddSection(sectionName);
        ++sec_index;
    }

    section_t_xml* pSection = m_sectionVec[sec_index];
    std::size_t len1 = strlen(val) + 1;
    char *new_val = NEW char[len1];
    strncpy(new_val, val, len1);
    std::size_t len2 = strlen(item) + 1;
    char *new_name = NEW char[len2];
    strncpy(new_name, item, len2);
    std::size_t hash = utilstring::hash_value(item);
    item_t_xml *pItem = NEW item_t_xml(new_name, hash, new_val, true, true);
    m_itemVec.push_back(pItem);
    
    pSection->m_nCount++;
    for (std::size_t i = sec_index + 1; i < m_sectionVec.size(); ++i)
    {
        section_t_xml *pTmp = m_sectionVec[i];
        if (nullptr != pTmp)
        {
            pTmp->m_nStart++;
        }
    }
    return true;
}

const char* CXmlFile::GetSectionNameByIndex(std::size_t section_index) const
{
    const section_t_xml* pSection = GetSectionByIndex(section_index);
    if (nullptr == pSection)
    {
        return "";
    }

    return pSection->m_strName;
}

char* CXmlFile::GetItemByIndex(std::size_t section_index, const char* item) const
{
	BOOST_ASSERT(nullptr == item);
	const section_t_xml* pSection = GetSectionByIndex(section_index);
	if (nullptr == pSection)
	{
		return "";
	}
	const item_t_xml* pItem = GetItem(pSection->m_strName, item);
	if (nullptr == pItem)
	{
		return "";
	}
	return pItem->m_strValue;
}

bool CXmlFile::FileFormLoad()
 {
	std::string err_LogManager;
	return FileFormLord(err_LogManager);
}
void CXmlFile::ReleaseRes()
{
	Release();
}

void CXmlFile::SetFileName(const char* strFileName)
{
    m_strFileName = strFileName;
}

std::size_t CXmlFile::GetItemIndex(const char* section, const char* item) const
{
    BOOST_ASSERT(nullptr == section);
    BOOST_ASSERT(nullptr == item);
    const section_t_xml* pSection = GetSection(section);
    if (nullptr == pSection)
    {
        return 0;
    }

    std::size_t nCount = pSection->m_nStart + pSection->m_nCount;
    for (std::size_t i = pSection->m_nStart; i < nCount; i++)
    {
        item_t_xml* pItem = m_itemVec[i];
        std::size_t hash = utilstring::hash_value(item);
        if (nullptr != pItem
            && pItem->m_nHash == hash
            && 0 == strcmp(pItem->m_strName, item))
        {
            return i;
        }
    }

    return 0;
}

std::size_t CXmlFile::GetItemIndex(const std::size_t section_index, const char* item) const
{
    BOOST_ASSERT(0 == section_index);
    BOOST_ASSERT(nullptr == item);
    const section_t_xml* pSection = GetSectionByIndex(section_index);
    if (nullptr == pSection)
    {
        return 0;
    }

    for (std::size_t i = pSection->m_nStart; i < pSection->m_nCount; i++)
    {
        item_t_xml* pItem = m_itemVec[i];
        std::size_t hash = utilstring::hash_value(item);
        if (nullptr != pItem
            && pItem->m_nHash == hash
            && STRING_EQUIP(pItem->m_strName, item))
        {
            return i;
        }
    }

    return 0;
}

std::size_t CXmlFile::GetSectionCount()
{
    return m_sectionMap.size();
}

std::size_t CXmlFile::GetItemCount(const char* section) const
{
    const section_t_xml *pSection = GetSection(section);
    if(nullptr == pSection)
    {
        return 0;
    }

    return (pSection->m_nCount);
}

std::size_t CXmlFile::GetItemCountByIndex(const std::size_t section_index) const
{
    const section_t_xml *pSection = GetSectionByIndex(section_index);
    if (nullptr == pSection)
    {
        return 0;
    }

    return (pSection->m_nCount);
}

//char* CXmlFile::GetItemName(const char* section, const char* item)
//{
//    std::size_t sec_index = 0;
//    if (!FindSectionIndex(section, sec_index))
//    {
//        return "";
//    }
//
//    section_t_xml *pSection = m_sectionVec[sec_index];
//    std::size_t nCount = pSection->m_nStart + pSection->m_nCount;
//	for (std::size_t i = pSection->m_nStart; i < nCount; ++i)
//    {
//        item_t_xml* pItem = m_itemVec[i];
//        std::size_t hash = utilstring::hash_value(item);
//        if (nullptr != pItem
//            && pItem->m_nHash == hash
//            && 0 == strcmp(pItem->m_strName, item))
//        {
//            return pItem->m_strName;
//        }
//    }
//
//    return "";
//}

char* CXmlFile::GetItemName(const char* section, int item_index) const
{
	std::size_t sec_index = 0;
	if (!FindSectionIndex(section, sec_index))
	{
		return "";
	}

	section_t_xml *pSection = m_sectionVec[sec_index];
	std::size_t nCount = pSection->m_nStart + item_index;

	if (nCount < 0 || nCount >= m_itemVec.size())
	{
		return "";
	}
	item_t_xml* pItem = m_itemVec[nCount];
	if (nullptr == pItem)
	{
		return "";
	}

	return pItem->m_strName;
}

//char* CXmlFile::GetItemValue(const char* section, const char* item)
//{
//    std::size_t sec_index = 0;
//    if (!FindSectionIndex(section, sec_index))
//    {
//        return "";
//    }
//
//    section_t_xml *pSection = m_sectionVec[sec_index];
//    std::size_t nCount = pSection->m_nStart + pSection->m_nCount;
//	for (std::size_t i = pSection->m_nStart; i < nCount; ++i)
//    {
//        item_t_xml* pItem = m_itemVec[i];
//        std::size_t hash = utilstring::hash_value(item);
//        if (nullptr != pItem
//            && pItem->m_nHash == hash
//            && 0 == strcmp(pItem->m_strName, item))
//        {
//            return pItem->m_strValue;
//        }
//    }
//
//    return "";
//}

char* CXmlFile::GetItemValue(const char* section, int item_index) const
{
	std::size_t sec_index = 0;
	if (!FindSectionIndex(section, sec_index))
	{
		return "";
	}

	section_t_xml *pSection = m_sectionVec[sec_index];
	std::size_t nCount = pSection->m_nStart + item_index;

	if (nCount < 0 || nCount >= m_itemVec.size())
	{
		return "";
	}
	item_t_xml* pItem = m_itemVec[nCount];
	if (nullptr == pItem)
	{
		return "";
	}

	return pItem->m_strValue;
}

void CXmlFile::SaveToFile()
{
    rapidxml::xml_document<> doc;
    rapidxml::xml_node<char> *pNodeRoot =
        doc.allocate_node(rapidxml::node_element, "Object");
    doc.append_node(pNodeRoot);
    for (std::size_t i = 0; i < m_sectionVec.size(); ++i)
    {
        rapidxml::xml_node<char> *pPrpety = 
            doc.allocate_node(rapidxml::node_element, "Propety");
        pNodeRoot->append_node(pPrpety);

        pPrpety->append_attribute(
            doc.allocate_attribute("ID", m_sectionVec[i]->m_strName));
        std::size_t nCount = m_sectionVec[i]->m_nStart + m_sectionVec[i]->m_nCount;
        for (std::size_t j = m_sectionVec[i]->m_nStart; j < nCount; ++j)
        {
            pPrpety->append_attribute(
                doc.allocate_attribute(m_itemVec[j]->m_strName, m_itemVec[j]->m_strValue));
        }
    }

    std::ofstream out(m_strFileName.c_str());
    out << "<!--xml version = '1.0' encoding = 'utf-8'-->\t\n";
    out << doc;
    out.close();
}

bool CXmlFile::QueryBool(const char* section, const char* item, bool def) const
{
    BOOST_ASSERT(nullptr == section);
    BOOST_ASSERT(nullptr == item);
    const char* value = GetData(section, item);
    if ('\0' == value[0])
    {
        return def;
    }

    return STRING_EQUIP(value, "true");
}

int CXmlFile::Queryint(const char* section, const char* item, int def)  const
{
	BOOST_ASSERT(nullptr == section);
	BOOST_ASSERT(nullptr == item);
	const char* value = GetData(section, item);
	if ('\0' == value[0])
	{
        return def;
	}

	return utilstring::as_int(value);
}

bool CXmlFile::QueryBool(const std::size_t section_index, const char* item, bool def) const
{
    const char* value = GetItemByIndex(section_index, item);
    if ('\0' == value[0])
    {
        return def;
    }

    return STRING_EQUIP(value, "true");
}

int CXmlFile::Queryint(const std::size_t section_index, const char* item, int def) const
{
    const char* value = GetItemByIndex(section_index, item);
    if ('\0' == value[0])
    {
        return def;
    }

    return utilstring::as_int(value);
}

int64_t CXmlFile::QueryInt64(const char* section, const char* item, int64_t def) const
{
	BOOST_ASSERT(nullptr == section);
	BOOST_ASSERT(nullptr == item);
	const char* value = GetData(section, item);
	if ('\0' == value[0])
	{
        return def;
	}

    return utilstring::as_int64(value);
}

int64_t CXmlFile::QueryInt64(const std::size_t section_index, const char* item, int64_t def) const
{
    const char* value = GetItemByIndex(section_index, item);
    if ('\0' == value[0])
    {
        return def;
    }

    return utilstring::as_int64(value);
}

float CXmlFile::QueryFloat(const char* section, const char* item, float def) const
{
	BOOST_ASSERT(nullptr == section);
	BOOST_ASSERT(nullptr == item);
	const char* value = GetData(section, item);
	if ('\0' == value[0])
	{
        return def;
	}

    return utilstring::as_float(value);
}

float CXmlFile::QueryFloat(const std::size_t section_index, const char* item, float def) const
{
    const char* value = GetItemByIndex(section_index, item);
    if ('\0' == value[0])
    {
        return def;
    }

    return utilstring::as_float(value);
}

double CXmlFile::QueryDouble(const char* section, const char* item, double def) const
{
	BOOST_ASSERT(nullptr == section);
	BOOST_ASSERT(nullptr == item);
	const char* value = GetData(section, item);
	if ('\0' == value[0])
	{
        return def;
	}

    return utilstring::as_double(value);
}

double CXmlFile::QueryDouble(const std::size_t section_index, const char* item, double def) const
{
    const char* value = GetItemByIndex(section_index, item);
    if ('\0' == value[0])
    {
        return def;
    }

    return utilstring::as_double(value);
}

char* CXmlFile::QueryString(const char* section, const char* item, char* def) const
{
	BOOST_ASSERT(nullptr == section);
	BOOST_ASSERT(nullptr == item);
	const char* value = GetData(section, item);
	if ('\0' == value[0])
	{
        return def;
	}

	return const_cast<char*>(value);
}

char* CXmlFile::QueryString(const std::size_t section_index, const char* item, char* def) const
{
    const char* value = GetItemByIndex(section_index, item);
    if ('\0' == value[0])
    {
        return def;
    }

    return const_cast<char*>(value);
}

bool CXmlFile::SetBool(const char* section, const char* item, const bool val, bool def /*= true*/)
{
    item_t_xml *pItem = GetItem(section, item);
    char* sz_val;
    if (val)
    {
        sz_val = "true";
    } 
    else
    {
        sz_val = "false";
    }

    if (nullptr != pItem && !def)
    {
        AddData(section, item, sz_val);
    }
    else
    {
        if (nullptr != pItem && pItem->m_bDelValue)
        {
            delete pItem->m_strValue;
        }
        std::size_t len = strlen(sz_val) + 1;
        char *new_val = NEW char[len];
        strncpy(new_val, sz_val, len);
        pItem->m_strName = new_val;
        pItem->m_bDelValue = true;
    }
    return true;
}

bool CXmlFile::Setint(const char* section, const char* item, const int val, bool def /*= true*/)
{
    item_t_xml *pItem = GetItem(section, item);
    char sz_val[64] = {};
    STX_SPRINTF(sz_val, "%d", val);
    if (nullptr != pItem && !def)
    {
        AddData(section, item, sz_val);
    }
    else
    {
        if (nullptr != pItem && pItem->m_bDelValue)
        {
            delete pItem->m_strValue;
        }
        std::size_t len = strlen(sz_val) + 1;
        char *new_val = NEW char[len];
        strncpy(new_val, sz_val, len);
        pItem->m_strName = new_val;
        pItem->m_bDelValue = true;
    }
    return true;
}

bool CXmlFile::Setint64(const char* section, const char* item, const int64_t val, bool def /*= true*/)
{
    item_t_xml *pItem = GetItem(section, item);
    char sz_val[64] = {};
    STX_SPRINTF(sz_val, "%lld", val);
    if (nullptr != pItem && !def)
    {
        AddData(section, item, sz_val);
    }
    else
    {
        if (nullptr != pItem && pItem->m_bDelValue)
        {
            delete pItem->m_strValue;
        }
        std::size_t len = strlen(sz_val) + 1;
        char *new_val = NEW char[len];
        strncpy(new_val, sz_val, len);
        pItem->m_strName = new_val;
        pItem->m_bDelValue = true;
    }
    return true;
}

bool CXmlFile::SetFloat(const char* section, const char* item, const float val, bool def /*= true*/)
{
    item_t_xml *pItem = GetItem(section, item);
    char sz_val[32] = {};
    STX_SPRINTF(sz_val, "%f", val);
    if (nullptr != pItem && !def)
    {
        AddData(section, item, sz_val);
    }
    else
    {
        if (nullptr != pItem && pItem->m_bDelValue)
        {
            delete pItem->m_strValue;
        }
        std::size_t len = strlen(sz_val) + 1;
        char *new_val = NEW char[len];
        strncpy(new_val, sz_val, len);
        pItem->m_strName = new_val;
        pItem->m_bDelValue = true;
    }
    return true;
}

bool CXmlFile::SetDouble(const char* section, const char* item, const double val, bool def /*= true*/)
{
    item_t_xml *pItem = GetItem(section, item);
    char sz_val[64] = {};
    STX_SPRINTF(sz_val, "%lf", val);
    if (nullptr != pItem && !def)
    {
        AddData(section, item, sz_val);
    }
    else
    {
        if (nullptr != pItem && pItem->m_bDelValue)
        {
            delete pItem->m_strValue;
        }
        std::size_t len = strlen(sz_val) + 1;
        char *new_val = NEW char[len];
        strncpy(new_val, sz_val, len);
        pItem->m_strName = new_val;
        pItem->m_bDelValue = true;
    }
    return true;
}

bool CXmlFile::SetString(const char* section, const char* item, const char* val, bool def /*= true*/)
{
    item_t_xml *pItem = GetItem(section, item);
    if (nullptr != pItem && !def)
    {
        AddData(section, item, val);
    } 
    else
    {
        if (nullptr != pItem && pItem->m_bDelValue)
        {
            delete pItem->m_strValue;
        }
        std::size_t len = strlen(val) + 1;
        char *new_val = NEW char[len];
        strncpy(new_val, val, len);
        pItem->m_strName = new_val;
        pItem->m_bDelValue = true;
    }
    return true;
}

bool CXmlFile::GetSectionList(const char* section, IVarList& args)
{
	for (std::size_t i = 0; i < m_sectionVec.size(); i++)
	{
		args.AddString(m_sectionVec[i]->m_strName);
	}
	return args.Empty();
}

bool CXmlFile::FindSectionIndex(const char* section, std::size_t& sec_index) const
{
    std::size_t hash = utilstring::hash_value(section);
    std::size_t nCount = m_sectionVec.size();
    for (std::size_t i = 0; i < nCount; i++)
    {
        section_t_xml *pSection = m_sectionVec[i];
        if (nullptr != pSection &&
            pSection->m_nHash == hash &&
            0 == stricmp(pSection->m_strName, section))
        {
            sec_index = i;
            return true;
        }
    }
    
    return false;
}

bool CXmlFile::FindSection(const char* section) const
{
	BOOST_ASSERT(section == nullptr);
	return (GetSection(section) != nullptr);
}

bool CXmlFile::DeleteSection(const char* section)
{
    std::vector<section_t_xml*>::iterator sec_eraser;
    std::size_t sec_index = 0;
    if (!FindSectionIndex(section, sec_index))
    {
        return false;
    }

    section_t_xml *pSection = m_sectionVec[sec_index];
    std::size_t nCount = pSection->m_nStart + pSection->m_nCount;
    std::vector<item_t_xml*>::iterator item_t_beg 
        = m_itemVec.begin() + pSection->m_nStart;
    std::vector<item_t_xml*>::iterator item_t_end
        = m_itemVec.begin() + nCount;
    for (std::size_t i = sec_index + 1; i < m_sectionVec.size(); ++i)
    {
        section_t_xml *pTmp = m_sectionVec[i];
        if (nullptr != pTmp)
        {
            pTmp->m_nStart -= nCount;
        }
    }

    for (std::size_t i = pSection->m_nStart; i < nCount; ++i)
    {
        item_t_xml *pItem = m_itemVec[i];
        if (nullptr != pItem)
        {
            delete pItem;
            pItem = nullptr;
        }
    }
    m_itemVec.erase(item_t_beg, item_t_end);

    sec_eraser = m_sectionVec.begin() + sec_index;
    delete pSection;
    pSection = nullptr;
    m_sectionVec.erase(sec_eraser);
    return true;
}

bool CXmlFile::DeleteItem(const char* section, const char* item_name)
{
    std::size_t sec_index = 0;
    if (!FindSectionIndex(section, sec_index))
    {
        return false;
    }
    section_t_xml *pSection = m_sectionVec[sec_index];
    if (nullptr != pSection)
    {
        return false;
    }

    std::size_t hash = utilstring::hash_value(item_name);
    std::size_t nCount = pSection->m_nStart + pSection->m_nCount;
    for (std::size_t i = pSection->m_nStart; i < nCount; ++i)
    {
        item_t_xml *pItem = m_itemVec[i];
        if (nullptr != pItem
            && pItem->m_nHash == hash
            && 0 == strcmp(pItem->m_strName, item_name))
        {
            //在pSection->m_nStart删除一个item，后续的section都妖减1
            for (std::size_t j = sec_index + 1; j < m_sectionVec.size(); ++j)
            {
                section_t_xml* pTmp = m_sectionVec[j];
                if (nullptr != pSection)
                {
                    pTmp->m_nStart--;
                }
            }
            std::vector<item_t_xml*>::iterator it =
                m_itemVec.begin() + i;
            m_itemVec.erase(it);

            //删除内存
            delete pItem;
            pItem = nullptr;

            pSection->m_nCount--;
            return true;
        }
    }

    return true;
}
