#ifndef _RAPID_DOCUMENT_H_
#define _RAPID_DOCUMENT_H_
#pragma once 
#include "../public/predefine.h"

#include <iosfwd>
#include <vector>
#include "../public/C++17/string_view.h"
#include <boost/noncopyable.hpp>

namespace rapidxml
{
	template <typename Ch>
	class xml_node;
	template <typename Ch>
	class xml_attribute;
	template <typename Ch>
	class xml_document;
}

enum XMLNodeType
{
	XNT_Document,
	XNT_Element,
	XNT_Data,
	XNT_CData,
	XNT_Comment,
	XNT_Declaration,
	XNT_Doctype,
	XNT_PI
};

// 根节点
class XMLDocument : boost::noncopyable
{
public:
	XMLDocument();

	XMLNodePtr Parse(ResIdentifierPtr const & source);

	void RootNode(XMLNodePtr const & new_node);

	void Print(std::ostream& os);

	XMLNodePtr CloneNode(XMLNodePtr const & node);
	XMLNodePtr AllocNode(XMLNodeType type, std::string_view name);
	XMLAttributePtr AllocAttribInt(std::string_view name, int32_t value);
	XMLAttributePtr AllocAttribUInt(std::string_view name, uint32_t value);
	XMLAttributePtr AllocAttribFloat(std::string_view name, float value);
	XMLAttributePtr AllocAttribString(std::string_view name, std::string_view value);
private:
	std::shared_ptr<rapidxml::xml_document<char>> m_doc;
	std::vector<char> m_pXmlValue;
	XMLNodePtr m_root;
};

// 子节点
class XMLNode : boost::noncopyable
{
	friend class XMLDocument;

public:
	explicit XMLNode(rapidxml::xml_node<char>* node);
	XMLNode(rapidxml::xml_document<char>& doc, XMLNodeType type, std::string_view name);

	std::string_view Name() const;
	XMLNodeType Type() const;
	XMLNodePtr Parent() const;

	XMLAttributePtr FirstAttrib(std::string_view name) const;
	XMLAttributePtr LastAttrib(std::string_view name) const;
	XMLAttributePtr FirstAttrib() const;
	XMLAttributePtr LastAttrib() const;

	XMLAttributePtr Attrib(std::string_view name) const;

	bool TryConvertAttrib(std::string_view name, int32_t& val, int32_t default_val) const;
	bool TryConvertAttrib(std::string_view name, uint32_t& val, uint32_t default_val) const;
	bool TryConvertAttrib(std::string_view name, float& val, float default_val) const;

	int32_t AttribInt(std::string_view name, int32_t default_val) const;
	uint32_t AttribUInt(std::string_view name, uint32_t default_val) const;
	float AttribFloat(std::string_view name, float default_val) const;
	std::string_view AttribString(std::string_view name, std::string_view default_val) const;

	XMLNodePtr FirstNode(std::string_view name) const;
	XMLNodePtr LastNode(std::string_view name) const;
	XMLNodePtr FirstNode() const;
	XMLNodePtr LastNode() const;

	XMLNodePtr PrevSibling(std::string_view name) const;
	XMLNodePtr NextSibling(std::string_view name) const;
	XMLNodePtr PrevSibling() const;
	XMLNodePtr NextSibling() const;

	void InsertNode(XMLNodePtr const & location, XMLNodePtr const & new_node);
	void InsertAttrib(XMLAttributePtr const & location, XMLAttributePtr const & new_attr);
	void AppendNode(XMLNodePtr const & new_node);
	void AppendAttrib(XMLAttributePtr const & new_attr);

	void RemoveNode(XMLNodePtr const & node);
	void RemoveAttrib(XMLAttributePtr const & attr);

	bool TryConvert(int32_t& val) const;
	bool TryConvert(uint32_t& val) const;
	bool TryConvert(float& val) const;

	int32_t ValueInt() const;
	uint32_t ValueUInt() const;
	float ValueFloat() const;
	std::string_view ValueString() const;

private:
	std::string_view m_strName;

	rapidxml::xml_node<char>* m_pNode;
	std::vector<XMLNodePtr> m_pChild;
	std::vector<XMLAttributePtr> m_pAttr;
};

// 存储数据节点
class XMLAttribute : boost::noncopyable
{
	friend class XMLDocument;
	friend class XMLNode;

public:
	explicit XMLAttribute(rapidxml::xml_attribute<char>* attr);
	XMLAttribute(rapidxml::xml_document<char>& doc, std::string_view name, std::string_view value);

	std::string_view Name() const;

	XMLAttributePtr NextAttrib(std::string_view name) const;
	XMLAttributePtr NextAttrib() const;

	bool TryConvert(int32_t& val) const;
	bool TryConvert(uint32_t& val) const;
	bool TryConvert(float& val) const;

	int32_t ValueInt() const;
	uint32_t ValueUInt() const;
	float ValueFloat() const;
	std::string_view ValueString() const;

private:
	rapidxml::xml_attribute<char>* m_pAttr;
	std::string_view m_szName;
	std::string_view m_szValue;
};
#endif//_RAPID_DOCUMENT_H_