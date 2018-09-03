#include "RenderMaterial.h"
#include "../System/ResLoader.h"
#include "../Util/UtilTool.h"
#include "../Container/Hash.h"
#include "../Tool/XMLDocument.h"

#include "../Container/C++17/filesystem.h"
#include <boost/assert.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
template <int N>
void ExtractFVector(std::string_view value_str, float* v)
{
	std::vector<std::string> strs;
	boost::algorithm::split(strs, value_str, boost::is_any_of(" "));
	for (size_t i = 0; i < N; ++i)
	{
		if (i < strs.size())
		{
			boost::algorithm::trim(strs[i]);
			v[i] = static_cast<float>(atof(strs[i].c_str()));
		}
		else
		{
			v[i] = 0;
		}
	}
}

class MaterialLoadingDesc : public ResLoadingDesc
{
	struct MaterialData
	{
		std::string strName;
		float4 f4Albedo;
		float fMetalness;
		float fGlossiness;
		float3 f3Emissive;
		std::array<std::string, RenderMaterial::TS_TypeCount> TexNames;

		MaterialData()
			:f4Albedo(0,0,0,0),fMetalness(0), fGlossiness(0), f3Emissive(0,0,0)
		{}
	};

	struct RenderMaterialDesc
	{
		XMLNodePtr pFileParse;
		std::string strResName;
		std::shared_ptr<MaterialData> MtlData;
		std::shared_ptr<RenderMaterialPtr> Mtl;
	};
public:
	MaterialLoadingDesc(const std::string strFileName, XMLNodePtr pFileParse)
	{
		m_MtlDesc.pFileParse = pFileParse;
		m_MtlDesc.strResName = strFileName;
		m_MtlDesc.MtlData = MakeSharedPtr<MaterialData>();
		m_MtlDesc.Mtl = MakeSharedPtr<RenderMaterialPtr>();
	}

	uint64_t Type() const override
	{
		static uint64_t const type = CT_HASH("MaterialLoadingDesc");
		return type;
	}

	bool StateLess() const override
	{
		return true;
	}

	void SubThreadStage() override
	{
		std::lock_guard<std::mutex> lock(m_MainThreadStageMutex);
		if (*m_MtlDesc.Mtl)
		{
			return;
		}

		XMLNodePtr root = m_MtlDesc.pFileParse;
		if (nullptr == root)
		{
			ResIdentifierPtr mtl_input = ResLoader::Instance()->Open(m_MtlDesc.strResName);
			XMLDocument doc;
			XMLNodePtr root = doc.Parse(mtl_input);
		}

		{
			XMLAttributePtr attr = root->Attrib("name");
			if (attr)
			{
				m_MtlDesc.MtlData->strName = std::string(attr->ValueString());
			}
			else
			{
				std::filesystem::path strResPath(m_MtlDesc.strResName);
				m_MtlDesc.MtlData->strName = strResPath.stem().string();
			}
		}

		XMLNodePtr albedo_node = root->FirstNode("albedo");
		if (albedo_node)
		{
			XMLAttributePtr attr = albedo_node->Attrib("color");
			if (attr)
			{
				ExtractFVector<4>(attr->ValueString(), &m_MtlDesc.MtlData->f4Albedo[0]);
			}
			attr = albedo_node->Attrib("texture");
			if (attr)
			{
				m_MtlDesc.MtlData->TexNames[RenderMaterial::TS_Albedo] = std::string(attr->ValueString());
			}
		}

		XMLNodePtr metalness_node = root->FirstNode("metalness");
		if (metalness_node)
		{
			XMLAttributePtr attr = metalness_node->Attrib("value");
			if (attr)
			{
				m_MtlDesc.MtlData->fMetalness = attr->ValueFloat();
			}
			attr = metalness_node->Attrib("texture");
			if (attr)
			{
				m_MtlDesc.MtlData->TexNames[RenderMaterial::TS_Metalness] = std::string(attr->ValueString());
			}
		}

		XMLNodePtr glossiness_node = root->FirstNode("glossiness");
		if (glossiness_node)
		{
			XMLAttributePtr attr = glossiness_node->Attrib("value");
			if (attr)
			{
				m_MtlDesc.MtlData->fGlossiness = attr->ValueFloat();
			}
			attr = glossiness_node->Attrib("texture");
			if (attr)
			{
				m_MtlDesc.MtlData->TexNames[RenderMaterial::TS_Glossiness] = std::string(attr->ValueString());
			}
		}

		XMLNodePtr emissive_node = root->FirstNode("emissive");
		if (emissive_node)
		{
			XMLAttributePtr attr = emissive_node->Attrib("color");
			if (attr)
			{
				ExtractFVector<3>(attr->ValueString(), &m_MtlDesc.MtlData->f3Emissive[0]);
			}
			attr = emissive_node->Attrib("texture");
			if (attr)
			{
				m_MtlDesc.MtlData->TexNames[RenderMaterial::TS_Emissive] = std::string(attr->ValueString());
			}
		}

		XMLNodePtr normal_node = root->FirstNode("normal");
		if (normal_node)
		{
			XMLAttributePtr attr = normal_node->Attrib("texture");
			if (attr)
			{
				m_MtlDesc.MtlData->TexNames[RenderMaterial::TS_Normal] = std::string(attr->ValueString());
			}
		}

		XMLNodePtr height_node = root->FirstNode("height");
		if (height_node)
		{
			XMLAttributePtr attr = height_node->Attrib("texture");
			if (attr)
			{
				m_MtlDesc.MtlData->TexNames[RenderMaterial::TS_Height] = std::string(attr->ValueString());
			}
		}
	}

	void MainThreadStage() override
	{
		std::lock_guard<std::mutex> lock(m_MainThreadStageMutex);
		this->MainThreadStageNoLock();
	}

	bool HasSubThreadStage() const override
	{
		return true;
	}

	bool Match(ResLoadingDesc const & rhs) const override
	{
		if (this->Type() == rhs.Type())
		{
			MaterialLoadingDesc const & mtlld = static_cast<const MaterialLoadingDesc&>(rhs);
			return (m_MtlDesc.strResName == mtlld.m_MtlDesc.strResName &&
						m_MtlDesc.pFileParse == mtlld.m_MtlDesc.pFileParse);
		}
		return false;
	}

	void CopyDataFrom(ResLoadingDesc const & rhs) override
	{
		BOOST_ASSERT(this->Type() == rhs.Type());

		MaterialLoadingDesc const & mtlld = static_cast<MaterialLoadingDesc const &>(rhs);
		m_MtlDesc.strResName = mtlld.m_MtlDesc.strResName;
		m_MtlDesc.MtlData = mtlld.m_MtlDesc.MtlData;
		m_MtlDesc.Mtl = mtlld.m_MtlDesc.Mtl;
	}

	std::shared_ptr<void> CloneResourceFrom(std::shared_ptr<void> const & resource) override
	{
		return resource;
	}

	std::shared_ptr<void> Resource() const override
	{
		return *m_MtlDesc.Mtl;
	}

private:
	void MainThreadStageNoLock()
	{
		if (!*m_MtlDesc.Mtl)
		{
			RenderMaterialPtr mtl = MakeSharedPtr<RenderMaterial>();
			mtl->m_strName = m_MtlDesc.MtlData->strName;
			mtl->m_f4Albedo = m_MtlDesc.MtlData->f4Albedo;
			mtl->m_fMetalness = m_MtlDesc.MtlData->fMetalness;
			mtl->m_fGlossiness = m_MtlDesc.MtlData->fGlossiness;
			mtl->m_f3Emissive = m_MtlDesc.MtlData->f3Emissive;
			mtl->m_TexNames = m_MtlDesc.MtlData->TexNames;

			*m_MtlDesc.Mtl = mtl;
		}
	}
private:
	RenderMaterialDesc m_MtlDesc;
	std::mutex m_MainThreadStageMutex;
};

RenderMaterialPtr SyncLoadRenderMaterial(const std::string strFileName, XMLNodePtr pFileParse /*= nullptr*/)
{
	return ResLoader::Instance()->SyncQueryT<RenderMaterial>(MakeSharedPtr<MaterialLoadingDesc>(strFileName, pFileParse));
}

RenderMaterialPtr ASyncLoadRenderMaterial(const std::string strFileName, XMLNodePtr pFileParse /*= nullptr*/)
{
	return ResLoader::Instance()->SyncQueryT<RenderMaterial>(MakeSharedPtr<MaterialLoadingDesc>(strFileName, pFileParse));
}
