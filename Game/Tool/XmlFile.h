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

    //�����ļ�
    bool FileFormLoad();

    //�ͷ���Դ
    void ReleaseRes();

    //�����ļ�·��
    void SetFileName(const char* strFileName);

    //��ȡ�ļ���
    const char* GetFileName() const;

    //��ȡ���ж�ֵ
    bool GetSectionList(const char* section, IVarList& args);
    
    //��ȡsection��Ӧ������
    bool FindSectionIndex(const char* section, std::size_t& sec_index) const;
    
    //��ȡitem��Ӧ������
    std::size_t GetItemIndex(const char* section, const char* item) const;
    std::size_t GetItemIndex(const std::size_t section_index, const char* item) const;
    
    //��ȡsection������
    std::size_t GetSectionCount();

    //��ȡ��Ӧsection��item������
    std::size_t GetItemCount(const char* section) const;
    std::size_t GetItemCountByIndex(const std::size_t section_index) const;
    
    //��ȡitem����
	char* GetItemName(const char* section, int item_index) const;
	//char* GetItemName(const char* section, const char* item);

    //��ȡitemֵ
	char* GetItemValue(const char* section, int item_index) const;
    //char* GetItemValue(const char* section, const char* item);

    //���浽�ļ�
    void SaveToFile();

    //ɾ��section
    bool DeleteSection(const char* section);

    //ɾ��item
    bool DeleteItem(const char* section, const char* item_name);
    
    //�鿴section����
    bool FindSection(const char* section) const;
    
    //��ȡ����
    const char* GetSectionNameByIndex(std::size_t section_index) const;

    //��ȡ��ֵ
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
    
    //���ü�ֵ�����ھ����ã�defΪtrue�Ҳ����ھ�����
    bool SetBool(const char* section, const char* item, const bool val, bool def = true);
    bool Setint(const char* section, const char* item, const int val, bool def = true);
    bool Setint64(const char* section, const char* item, const int64_t val, bool def = true);
    bool SetFloat(const char* section, const char* item, const float val, bool def = true);
    bool SetDouble(const char* section, const char* item, const double val, bool def = true);
    bool SetString(const char* section, const char* item, const char* val, bool def = true);

    //���Ӷ�
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
    //��ϣӳ��
    typedef std::multimap<std::size_t, std::size_t> sectionMapType;
    typedef sectionMapType::const_iterator sectionMapItem;
private:
    //��ȡsecion��item��Ӧ��itemֵ���ַ���
	char* GetData(const char* section, const char* item) const;
    //��ȡitem��ָ��
    item_t_xml* GetItem(const char* section, const char* item) const;
    char* GetItemByIndex(std::size_t section_index, const char* item) const;
    //��ȡseciotn��ָ��
    section_t_xml* GetSection(const char* section) const;
	section_t_xml* GetSectionByIndex(const std::size_t section_index) const;
    //����item
    bool AddData(const char* sectionName, const char* item, const char* val);
    //������ھ����ã������ھ�����
    bool SetData(const char* sectionName, const char* item, const char* val);
	//�ͷ��ڴ�
    void Release();
	//�ı������ڴ�
    bool FileFormLord(std::string err_LogManager);
private:
	std::string m_strFileName;//�ļ���
	char* m_pXmlValue;        //�ļ��ַ���
	rapidxml::xml_document<> m_doc;
	std::vector<section_t_xml*> m_sectionVec;//�ļ��μ���
	std::vector<item_t_xml*> m_itemVec;//�ļ����м�
	sectionMapType m_sectionMap;
};
#endif//XMLFILE_H