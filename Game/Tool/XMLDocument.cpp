#include "../Tool/XMLDocument.h"
#include "../SDK/rapid/rapidxml.hpp"
#include "../SDK/rapid/rapidxml_print.hpp"
#include "../Tool/ResLoader.h"
#include "../Util/UtilTool.h"
#include <boost/lexical_cast.hpp>

XMLDocument::XMLDocument()
	: m_doc(MakeSharedPtr<rapidxml::xml_document<char>>())
{
}

XMLNodePtr XMLDocument::Parse(ResIdentifierPtr const & source)
{
	source->seekg(0, std::ios_base::end);
	int len = static_cast<int>(source->tellg());
	source->seekg(0, std::ios_base::beg);
	m_pXmlValue.resize(len + 1, 0);
	source->read(&m_pXmlValue[0], len);

	m_doc->parse<0>(&m_pXmlValue[0]);
	m_root = MakeSharedPtr<XMLNode>(m_doc->first_node());

	return m_root;
}

void XMLDocument::Print(std::ostream& os)
{
	os << "<?xml version=\"1.0\"?>" << std::endl << std::endl;
	os << *m_doc;
}

XMLNodePtr XMLDocument::CloneNode(XMLNodePtr const & node)
{
	return MakeSharedPtr<XMLNode>(m_doc->clone_node(node->m_pNode));
}

XMLNodePtr XMLDocument::AllocNode(XMLNodeType type, std::string_view name)
{
	return MakeSharedPtr<XMLNode>(*m_doc, type, name);
}

XMLAttributePtr XMLDocument::AllocAttribInt(std::string_view name, int32_t value)
{
	return this->AllocAttribString(name, std::to_string(value));
}

XMLAttributePtr XMLDocument::AllocAttribUInt(std::string_view name, uint32_t value)
{
	return this->AllocAttribString(name, std::to_string(value));
}

XMLAttributePtr XMLDocument::AllocAttribFloat(std::string_view name, float value)
{
	return this->AllocAttribString(name, std::to_string(value));
}

XMLAttributePtr XMLDocument::AllocAttribString(std::string_view name, std::string_view value)
{
	return MakeSharedPtr<XMLAttribute>(*m_doc, name, value);
}

void XMLDocument::RootNode(XMLNodePtr const & new_node)
{
	m_doc->remove_all_nodes();
	m_doc->append_node(new_node->m_pNode);
	m_root = new_node;
}


XMLNode::XMLNode(rapidxml::xml_node<char>* node)
	: m_pNode(node)
{
	if (m_pNode != nullptr)
	{
		m_strName = std::string_view(m_pNode->name(), m_pNode->name_size());
	}
}

XMLNode::XMLNode(rapidxml::xml_document<char>& doc, XMLNodeType type, std::string_view name)
	: m_strName(name)
{
	rapidxml::node_type xtype;
	switch (type)
	{
	case XNT_Document:
		xtype = rapidxml::node_document;
		break;

	case XNT_Element:
		xtype = rapidxml::node_element;
		break;

	case XNT_Data:
		xtype = rapidxml::node_data;
		break;

	case XNT_CData:
		xtype = rapidxml::node_cdata;
		break;

	case XNT_Comment:
		xtype = rapidxml::node_comment;
		break;

	case XNT_Declaration:
		xtype = rapidxml::node_declaration;
		break;

	case XNT_Doctype:
		xtype = rapidxml::node_doctype;
		break;

	case XNT_PI:
	default:
		xtype = rapidxml::node_pi;
		break;
	}

	m_pNode = doc.allocate_node(xtype, name.data(), nullptr, name.size());
}

std::string_view XMLNode::Name() const
{
	return m_strName;
}

XMLNodeType XMLNode::Type() const
{
	switch (m_pNode->type())
	{
	case rapidxml::node_document:
		return XNT_Document;

	case rapidxml::node_element:
		return XNT_Element;

	case rapidxml::node_data:
		return XNT_Data;

	case rapidxml::node_cdata:
		return XNT_CData;

	case rapidxml::node_comment:
		return XNT_Comment;

	case rapidxml::node_declaration:
		return XNT_Declaration;

	case rapidxml::node_doctype:
		return XNT_Doctype;

	case rapidxml::node_pi:
	default:
		return XNT_PI;
	}
}

XMLNodePtr XMLNode::Parent() const
{
	auto* node = m_pNode->parent();
	if (node)
	{
		return MakeSharedPtr<XMLNode>(node);
	}
	else
	{
		return XMLNodePtr();
	}
}

XMLAttributePtr XMLNode::FirstAttrib(std::string_view name) const
{
	auto* attr = m_pNode->first_attribute(name.data(), name.size());
	if (attr)
	{
		return MakeSharedPtr<XMLAttribute>(attr);
	}
	else
	{
		return XMLAttributePtr();
	}
}

XMLAttributePtr XMLNode::LastAttrib(std::string_view name) const
{
	auto* attr = m_pNode->last_attribute(name.data(), name.size());
	if (attr)
	{
		return MakeSharedPtr<XMLAttribute>(attr);
	}
	else
	{
		return XMLAttributePtr();
	}
}

XMLAttributePtr XMLNode::FirstAttrib() const
{
	auto* attr = m_pNode->first_attribute();
	if (attr)
	{
		return MakeSharedPtr<XMLAttribute>(attr);
	}
	else
	{
		return XMLAttributePtr();
	}
}

XMLAttributePtr XMLNode::LastAttrib() const
{
	auto* attr = m_pNode->last_attribute();
	if (attr)
	{
		return MakeSharedPtr<XMLAttribute>(attr);
	}
	else
	{
		return XMLAttributePtr();
	}
}

XMLAttributePtr XMLNode::Attrib(std::string_view name) const
{
	return this->FirstAttrib(name);
}

bool XMLNode::TryConvertAttrib(std::string_view name, int32_t& val, int32_t default_val) const
{
	val = default_val;

	auto attr = this->Attrib(name);
	return attr ? attr->TryConvert(val) : true;
}

bool XMLNode::TryConvertAttrib(std::string_view name, uint32_t& val, uint32_t default_val) const
{
	val = default_val;

	auto attr = this->Attrib(name);
	return attr ? attr->TryConvert(val) : true;
}

bool XMLNode::TryConvertAttrib(std::string_view name, float& val, float default_val) const
{
	val = default_val;

	auto attr = this->Attrib(name);
	return attr ? attr->TryConvert(val) : true;
}

int32_t XMLNode::AttribInt(std::string_view name, int32_t default_val) const
{
	auto attr = this->Attrib(name);
	return attr ? attr->ValueInt() : default_val;
}

uint32_t XMLNode::AttribUInt(std::string_view name, uint32_t default_val) const
{
	auto attr = this->Attrib(name);
	return attr ? attr->ValueUInt() : default_val;
}

float XMLNode::AttribFloat(std::string_view name, float default_val) const
{
	auto attr = this->Attrib(name);
	return attr ? attr->ValueFloat() : default_val;
}

std::string_view XMLNode::AttribString(std::string_view name, std::string_view default_val) const
{
	auto attr = this->Attrib(name);
	return attr ? attr->ValueString() : default_val;
}

XMLNodePtr XMLNode::FirstNode(std::string_view name) const
{
	auto* node = m_pNode->first_node(name.data(), name.size());
	if (node)
	{
		return MakeSharedPtr<XMLNode>(node);
	}
	else
	{
		return XMLNodePtr();
	}
}

XMLNodePtr XMLNode::LastNode(std::string_view name) const
{
	auto* node = m_pNode->last_node(name.data(), name.size());
	if (node)
	{
		return MakeSharedPtr<XMLNode>(node);
	}
	else
	{
		return XMLNodePtr();
	}
}

XMLNodePtr XMLNode::FirstNode() const
{
	auto* node = m_pNode->first_node();
	if (node)
	{
		return MakeSharedPtr<XMLNode>(node);
	}
	else
	{
		return XMLNodePtr();
	}
}

XMLNodePtr XMLNode::LastNode() const
{
	auto* node = m_pNode->last_node();
	if (node)
	{
		return MakeSharedPtr<XMLNode>(node);
	}
	else
	{
		return XMLNodePtr();
	}
}

XMLNodePtr XMLNode::PrevSibling(std::string_view name) const
{
	auto* node = m_pNode->previous_sibling(name.data(), name.size());
	if (node)
	{
		return MakeSharedPtr<XMLNode>(node);
	}
	else
	{
		return XMLNodePtr();
	}
}

XMLNodePtr XMLNode::NextSibling(std::string_view name) const
{
	auto* node = m_pNode->next_sibling(name.data(), name.size());
	if (node)
	{
		return MakeSharedPtr<XMLNode>(node);
	}
	else
	{
		return XMLNodePtr();
	}
}

XMLNodePtr XMLNode::PrevSibling() const
{
	auto* node = m_pNode->previous_sibling();
	if (node)
	{
		return MakeSharedPtr<XMLNode>(node);
	}
	else
	{
		return XMLNodePtr();
	}
}

XMLNodePtr XMLNode::NextSibling() const
{
	auto* node = m_pNode->next_sibling();
	if (node)
	{
		return MakeSharedPtr<XMLNode>(node);
	}
	else
	{
		return XMLNodePtr();
	}
}

void XMLNode::InsertNode(XMLNodePtr const & location, XMLNodePtr const & new_node)
{
	m_pNode->insert_node(location->m_pNode, new_node->m_pNode);
	for (size_t i = 0; i < m_pChild.size(); ++i)
	{
		if (m_pChild[i]->m_pNode == location->m_pNode)
		{
			m_pChild.insert(m_pChild.begin() + i, new_node);
			break;
		}
	}
}

void XMLNode::InsertAttrib(XMLAttributePtr const & location, XMLAttributePtr const & new_attr)
{
	m_pNode->insert_attribute(location->m_pAttr, new_attr->m_pAttr);
	for (size_t i = 0; i < m_pAttr.size(); ++i)
	{
		if (m_pAttr[i]->m_pAttr == location->m_pAttr)
		{
			m_pAttr.insert(m_pAttr.begin() + i, new_attr);
			break;
		}
	}
}

void XMLNode::AppendNode(XMLNodePtr const & new_node)
{
	m_pNode->append_node(new_node->m_pNode);
	m_pChild.push_back(new_node);
}

void XMLNode::AppendAttrib(XMLAttributePtr const & new_attr)
{
	m_pNode->append_attribute(new_attr->m_pAttr);
	m_pAttr.push_back(new_attr);
}

void XMLNode::RemoveNode(XMLNodePtr const & node)
{
	m_pNode->remove_node(node->m_pNode);
	for (size_t i = 0; i < m_pChild.size(); ++i)
	{
		if (m_pChild[i]->m_pNode == node->m_pNode)
		{
			m_pChild.erase(m_pChild.begin() + i);
			break;
		}
	}
}

void XMLNode::RemoveAttrib(XMLAttributePtr const & attr)
{
	m_pNode->remove_attribute(attr->m_pAttr);
	for (size_t i = 0; i < m_pAttr.size(); ++i)
	{
		if (m_pAttr[i]->m_pAttr == attr->m_pAttr)
		{
			m_pAttr.erase(m_pAttr.begin() + i);
			break;
		}
	}
}

bool XMLNode::TryConvert(int32_t& val) const
{
	return boost::conversion::try_lexical_convert(this->ValueString().data(), val);
}

bool XMLNode::TryConvert(uint32_t& val) const
{
	return boost::conversion::try_lexical_convert(this->ValueString().data(), val);
}

bool XMLNode::TryConvert(float& val) const
{
	return boost::conversion::try_lexical_convert(this->ValueString().data(), val);
}

int32_t XMLNode::ValueInt() const
{
	return std::stol(std::string(this->ValueString()));
}

uint32_t XMLNode::ValueUInt() const
{
	return std::stoul(std::string(this->ValueString()));
}

float XMLNode::ValueFloat() const
{
	return std::stof(std::string(this->ValueString()));
}

std::string_view XMLNode::ValueString() const
{
	return std::string_view(m_pNode->value(), m_pNode->value_size());
}


XMLAttribute::XMLAttribute(rapidxml::xml_attribute<char>* attr)
	: m_pAttr(attr)
{
	if (m_pAttr != nullptr)
	{
		auto const * xml_attr = m_pAttr;
		m_szName = std::string_view(xml_attr->name(), xml_attr->name_size());
		m_szValue = std::string_view(xml_attr->value(), xml_attr->value_size());
	}
}

XMLAttribute::XMLAttribute(rapidxml::xml_document<char>& doc, std::string_view name, std::string_view value)
	: m_szName(name), m_szValue(value)
{
	m_pAttr = doc.allocate_attribute(name.data(), value.data(), name.size(), value.size());
}

std::string_view XMLAttribute::Name() const
{
	return m_szName;
}

XMLAttributePtr XMLAttribute::NextAttrib(std::string_view name) const
{
	auto* attr = m_pAttr->next_attribute(name.data(), name.size());
	if (attr)
	{
		return MakeSharedPtr<XMLAttribute>(attr);
	}
	else
	{
		return XMLAttributePtr();
	}
}

XMLAttributePtr XMLAttribute::NextAttrib() const
{
	auto* attr = m_pAttr->next_attribute();
	if (attr)
	{
		return MakeSharedPtr<XMLAttribute>(attr);
	}
	else
	{
		return XMLAttributePtr();
	}
}

bool XMLAttribute::TryConvert(int32_t& val) const
{
	return boost::conversion::try_lexical_convert(m_szValue.data(), val);
}

bool XMLAttribute::TryConvert(uint32_t& val) const
{
	return boost::conversion::try_lexical_convert(m_szValue.data(), val);
}

bool XMLAttribute::TryConvert(float& val) const
{
	return boost::conversion::try_lexical_convert(m_szValue.data(), val);
}

int32_t XMLAttribute::ValueInt() const
{
	return std::stol(std::string(m_szValue));
}

uint32_t XMLAttribute::ValueUInt() const
{
	return std::stoul(std::string(m_szValue));
}

float XMLAttribute::ValueFloat() const
{
	return std::stof(std::string(m_szValue));
}

std::string_view XMLAttribute::ValueString() const
{
	return m_szValue;
}