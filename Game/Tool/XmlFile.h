#ifndef XMLFILE_H
#define XMLFILE_H
#pragma once 

#include "../public/i_varlist.h"
#include "../SDK/rapid/rapidxml.hpp"
#include "../SDK/rapid/rapidxml_print.hpp"

#include <vector>
#include <map>
#include <string>
#include <xutility>

class CXmlFile
{
	typedef std::map<std::string, std::string> ChiledElement;
public:
    CXmlFile(const char* file);

    ~CXmlFile();

    //载入文件
    bool FileFormLoad();

    //释放资源
    void ReleaseRes();

    //设置文件路径
    void SetFileName(const char* strFileName);

    //获取文件名
    const char* GetFileName() const;

    //获取所有段值
    bool GetSectionList(const char* section, IVarList& args);
    
    //获取section对应的索引
    bool FindSectionIndex(const char* section, std::size_t& sec_index) const;
    
    //获取item对应的索引
    std::size_t GetItemIndex(const char* section, const char* item) const;
    std::size_t GetItemIndex(const std::size_t section_index, const char* item) const;
    
    //获取section的数量
    std::size_t GetSectionCount();

    //获取对应section下item的数量
    std::size_t GetItemCount(const char* section) const;
    std::size_t GetItemCountByIndex(const std::size_t section_index) const;
    
    //获取item名字
	char* GetItemName(const char* section, int item_index) const;
	//char* GetItemName(const char* section, const char* item);

    //获取item值
	char* GetItemValue(const char* section, int item_index) const;
    //char* GetItemValue(const char* section, const char* item);

    //保存到文件
    void SaveToFile();

    //删除section
    bool DeleteSection(const char* section);

    //删除item
    bool DeleteItem(const char* section, const char* item_name);
    
    //查看section存在
    bool FindSection(const char* section) const;
    
    //获取段名
    const char* GetSectionNameByIndex(std::size_t section_index) const;

    //读取键值
    bool QueryBool(const char* section, const char* item, bool def) const;
    int Queryint(const char* section, const char* item, int def) const;
    int64_t QueryInt64(const char* section, const char* item, int64_t def) const;
    float QueryFloat(const char* section, const char* item, float def) const;
    double QueryDouble(const char* section, const char* item, double def) const;
    char* QueryString(const char* section, const char* item, char* def) const;
	
    bool QueryBool(const std::size_t section_index, const char* item, bool def) const;
    int Queryint(const std::size_t section_index, const char* item, int def) const;
    int64_t QueryInt64(const std::size_t section_index, const char* item, int64_t def) const;
    float QueryFloat(const std::size_t section_index, const char* item, float def) const;
    double QueryDouble(const std::size_t section_index, const char* item, double def) const;
    char* QueryString(const std::size_t section_index, const char* item, char* def) const;
    
    //设置键值，存在就设置，def为true且不存在就增加
    bool SetBool(const char* section, const char* item, const bool val, bool def = true);
    bool Setint(const char* section, const char* item, const int val, bool def = true);
    bool Setint64(const char* section, const char* item, const int64_t val, bool def = true);
    bool SetFloat(const char* section, const char* item, const float val, bool def = true);
    bool SetDouble(const char* section, const char* item, const double val, bool def = true);
    bool SetString(const char* section, const char* item, const char* val, bool def = true);

    //增加段
    bool AddSection(const char* sectionName);
private:
	struct section_t_xml
	{
		char* m_strName;
		std::size_t m_nHash;
		bool m_bDelName;
		std::size_t m_nStart;
		std::size_t m_nCount;

		section_t_xml(char* name, unsigned int hash,bool DelName = false)
			:m_strName(name), m_nHash(hash), m_bDelName(DelName), m_nStart(0), m_nCount(0)
		{}
		~section_t_xml()
		{
			if (m_bDelName)
			{
				delete[] m_strName;
			}
		}
	};
	struct item_t_xml
	{
		char* m_strName;
		char* m_strValue;
		unsigned int m_nHash;
		bool m_bDelName;
		bool m_bDelValue;

		item_t_xml(char* name, unsigned int hash, char*value,
					bool bDelName = false, bool bDelValue = false)
			:m_strName(name), m_nHash(hash), m_strValue(value),
			m_bDelName(bDelName), m_bDelValue(bDelValue)
		{}

		~item_t_xml()
		{
			if (m_bDelName)
			{
				delete[] m_strName;
			}
			if (m_bDelValue)
			{
				delete[] m_strValue;
			}
		}
	};
    //哈希映射
    typedef std::multimap<std::size_t, std::size_t> sectionMapType;
    typedef sectionMapType::const_iterator sectionMapItem;
private:
    //获取secion和item对应的item值的字符串
	char* GetData(const char* section, const char* item) const;
    //获取item的指针
    item_t_xml* GetItem(const char* section, const char* item) const;
    char* GetItemByIndex(std::size_t section_index, const char* item) const;
    //获取seciotn的指针
    section_t_xml* GetSection(const char* section) const;
	section_t_xml* GetSectionByIndex(const std::size_t section_index) const;
    //增加item
    bool AddData(const char* sectionName, const char* item, const char* val);
    //如果存在就设置，不存在就新增
    bool SetData(const char* sectionName, const char* item, const char* val);
	//释放内存
    void Release();
	//文本载入内存
    bool FileFormLord(std::string err_LogManager);
private:
	std::string m_strFileName;//文件名
	char* m_pXmlValue;        //文件字符串
	rapidxml::xml_document<> m_doc;
	std::vector<section_t_xml*> m_sectionVec;//文件段集合
	std::vector<item_t_xml*> m_itemVec;//文件所有键
	sectionMapType m_sectionMap;
};
#endif//XMLFILE_H