#include "Mesh.h"
#include "../System/ResLoader.h"
#include "../System/Log.h"
#include "../Platform/DxGraphDevice.h"
#include "../Util/UtilTool.h"
#include "../Container/Hash.h"
#include "../Render/SceneManager.h"
#include "../Render/ICamera.h"
#include "../Render/Material.h"
#include "../Render/ITexture.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "../SDK/tinyobj/tiny_obj_loader.h"

#include "../Container/C++17/filesystem.h"
typedef struct { float4 pos; float2 tc; Color color; float4 normal; } vertex_t;
vertex_t box_mesh[36] = {
	// Positions                  // Texture Coords  //color           //rhw // Normals
	{ { -0.5f, -0.5f, -0.5f, 1.0f },{ 0.0f,  0.0f },{ 1.0f, 0.2f, 0.2f, 1.0f },{ 0.0f,  0.0f,-1.0f,0.0f } },
{ { -0.5f,  0.5f, -0.5f, 1.0f },{ 0.0f,  1.0f },{ 1.0f, 0.2f, 0.2f, 1.0f },{ 0.0f,  0.0f,-1.0f,0.0f } },
{ { 0.5f,  0.5f, -0.5f, 1.0f },{ 1.0f,  1.0f },{ 1.0f, 0.2f, 0.2f, 1.0f },{ 0.0f,  0.0f,-1.0f ,0.0f } },
{ { 0.5f,  0.5f, -0.5f, 1.0f },{ 1.0f,  1.0f },{ 1.0f, 0.2f, 0.2f, 1.0f },{ 0.0f,  0.0f,-1.0f,0.0f } },
{ { 0.5f, -0.5f, -0.5f, 1.0f },{ 1.0f,  0.0f },{ 1.0f, 0.2f, 0.2f, 1.0f },{ 0.0f,  0.0f,-1.0f ,0.0f } },
{ { -0.5f, -0.5f, -0.5f, 1.0f },{ 0.0f,  0.0f },{ 1.0f, 0.2f, 0.2f, 1.0f },{ 0.0f,  0.0f,-1.0f,0.0f } },

{ { -0.5f, -0.5f,  0.5f, 1.0f },{ 0.0f,  0.0f },{ 0.2f, 1.0f, 0.2f, 1.0f },{ 0.0f,  0.0f, 1.0f,0.0f } },
{ { 0.5f, -0.5f,  0.5f, 1.0f },{ 1.0f,  0.0f },{ 0.2f, 1.0f, 0.2f, 1.0f },{ 0.0f,  0.0f,  1.0f,0.0f } },
{ { 0.5f,  0.5f,  0.5f, 1.0f },{ 1.0f,  1.0f },{ 0.2f, 1.0f, 0.2f, 1.0f },{ 0.0f,  0.0f,  1.0f,0.0f } },
{ { 0.5f,  0.5f,  0.5f, 1.0f },{ 1.0f,  1.0f },{ 0.2f, 1.0f, 0.2f, 1.0f },{ 0.0f,  0.0f,  1.0f,0.0f } },
{ { -0.5f,  0.5f,  0.5f, 1.0f },{ 0.0f,  1.0f },{ 0.2f, 1.0f, 0.2f, 1.0f },{ 0.0f,  0.0f,  1.0f,0.0f } },
{ { -0.5f, -0.5f,  0.5f, 1.0f },{ 0.0f,  0.0f },{ 0.2f, 1.0f, 0.2f, 1.0f },{ 0.0f,  0.0f,  1.0f,0.0f } },

{ { -0.5f,  0.5f,  0.5f, 1.0f },{ 1.0f,  0.0f },{ 0.2f, 0.2f, 1.0f, 1.0f },{ -1.0f,  0.0f,  0.0f,0.0f } },
{ { -0.5f,  0.5f, -0.5f, 1.0f },{ 1.0f,  1.0f },{ 0.2f, 0.2f, 1.0f, 1.0f },{ -1.0f,  0.0f,  0.0f,0.0f } },
{ { -0.5f, -0.5f, -0.5f, 1.0f },{ 0.0f,  1.0f },{ 0.2f, 0.2f, 1.0f, 1.0f },{ -1.0f,  0.0f,  0.0f,0.0f } },
{ { -0.5f, -0.5f, -0.5f, 1.0f },{ 0.0f,  1.0f },{ 0.2f, 0.2f, 1.0f, 1.0f },{ -1.0f,  0.0f,  0.0f,0.0f } },
{ { -0.5f, -0.5f,  0.5f, 1.0f },{ 0.0f,  0.0f },{ 0.2f, 0.2f, 1.0f, 1.0f },{ -1.0f,  0.0f,  0.0f,0.0f } },
{ { -0.5f,  0.5f,  0.5f,1.0f },{ 1.0f,  0.0f },{ 0.2f, 0.2f, 1.0f, 1.0f },{ -1.0f,  0.0f,  0.0f,0.0f } },

{ { 0.5f,  0.5f,  0.5f,1.0f },{ 1.0f,  0.0f },{ 1.0f, 0.2f, 1.0f, 1.0f },{ 1.0f,  0.0f,  0.0f,0.0f } },
{ { 0.5f, -0.5f,  0.5f,1.0f },{ 0.0f,  0.0f },{ 1.0f, 0.2f, 1.0f, 1.0f },{ 1.0f,  0.0f,  0.0f,0.0f } },
{ { 0.5f, -0.5f, -0.5f,1.0f },{ 0.0f,  1.0f },{ 1.0f, 0.2f, 1.0f, 1.0f },{ 1.0f,  0.0f,  0.0f,0.0f } },
{ { 0.5f, -0.5f, -0.5f,1.0f },{ 0.0f,  1.0f },{ 1.0f, 0.2f, 1.0f, 1.0f },{ 1.0f,  0.0f,  0.0f,0.0f } },
{ { 0.5f,  0.5f, -0.5f,1.0f },{ 1.0f,  1.0f },{ 1.0f, 0.2f, 1.0f, 1.0f },{ 1.0f,  0.0f,  0.0f,0.0f } },
{ { 0.5f,  0.5f,  0.5f,1.0f },{ 1.0f,  0.0f },{ 1.0f, 0.2f, 1.0f, 1.0f },{ 1.0f,  0.0f,  0.0f,0.0f } },

{ { -0.5f, -0.5f, -0.5f,1.0f },{ 0.0f,  1.0f },{ 1.0f, 1.0f, 0.2f, 1.0f },{ 0.0f, -1.0f,  0.0f,0.0f } },
{ { 0.5f, -0.5f, -0.5f,1.0f },{ 1.0f,  1.0f },{ 1.0f, 1.0f, 0.2f, 1.0f },{ 0.0f, -1.0f,  0.0f,0.0f } },
{ { 0.5f, -0.5f,  0.5f,1.0f },{ 1.0f,  0.0f },{ 1.0f, 1.0f, 0.2f, 1.0f },{ 0.0f, -1.0f,  0.0f,0.0f } },
{ { 0.5f, -0.5f,  0.5f,1.0f },{ 1.0f,  0.0f },{ 1.0f, 1.0f, 0.2f, 1.0f },{ 0.0f, -1.0f,  0.0f,0.0f } },
{ { -0.5f, -0.5f,  0.5f,1.0f },{ 0.0f,  0.0f },{ 1.0f, 1.0f, 0.2f, 1.0f },{ 0.0f, -1.0f,  0.0f,0.0f } },
{ { -0.5f, -0.5f, -0.5f,1.0f },{ 0.0f,  1.0f },{ 1.0f, 1.0f, 0.2f, 1.0f },{ 0.0f, -1.0f,  0.0f,0.0f } },

{ { -0.5f,  0.5f, -0.5f, 1.0f },{ 0.0f,  1.0f },{ 0.2f, 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f,  0.0f,0.0f } },
{ { -0.5f,  0.5f,  0.5f, 1.0f },{ 0.0f,  0.0f },{ 0.2f, 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f,  0.0f,0.0f } },
{ { 0.5f,  0.5f,  0.5f, 1.0f },{ 1.0f,  0.0f },{ 0.2f, 1.0f, 1.0f, 1.0f },{ 0.0f,1.0f,  0.0f,0.0f } },
{ { 0.5f,  0.5f,  0.5f, 1.0f },{ 1.0f,  0.0f },{ 0.2f, 1.0f, 1.0f, 1.0f },{ 0.0f,1.0f,  0.0f,0.0f } },
{ { 0.5f,  0.5f, -0.5f, 1.0f },{ 1.0f,  1.0f },{ 0.2f, 1.0f, 1.0f, 1.0f },{ 0.0f,1.0f,  0.0f,0.0f } },
{ { -0.5f,  0.5f, -0.5f, 1.0f },{ 0.0f,  1.0f },{ 0.2f, 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f,  0.0f,0.0f } }
};

class ModelLoadingDesc:public ResLoadingDesc
{
	struct MeshData
	{
		std::string strName;
		int32_t nMtlId;
		uint32_t nLods;
		std::vector<int3> Int3diceVec;
	};

	struct ModelData
	{
		std::vector<RenderMaterialPtr> MaterialVec;
		std::vector<MeshData> MeshVec;
		std::vector<zbVertex4D> VerticeVec;
	};

	struct ModelDesc
	{
		std::string strName;
		uint32_t nAttr;
		std::shared_ptr<ModelData> pModelData;
		std::shared_ptr<RenderModelPtr> pModel;
		std::function<RenderModelPtr(const std::wstring &, const std::string&)> CreateModelFactoryFunc;
		std::function<StaticMeshPtr(const RenderModelPtr&, const std::wstring&)> CreateMeshFactoryFunc;
	};

public:
	ModelLoadingDesc(const std::string& strName, uint32_t nAttr,
		std::function<RenderModelPtr(const std::wstring&, const std::string&)> CreateModelFactoryFunc,
		std::function<StaticMeshPtr(const RenderModelPtr&, const std::wstring&)> CreateMeshFactoryFunc)
	{
		model_desc_.strName = strName;
		model_desc_.nAttr = nAttr;
		model_desc_.CreateModelFactoryFunc = CreateModelFactoryFunc;
		model_desc_.CreateMeshFactoryFunc = CreateMeshFactoryFunc;
		model_desc_.pModelData = MakeSharedPtr<ModelData>();
		model_desc_.pModel = MakeSharedPtr<RenderModelPtr>();
	}

	virtual uint64_t Type() const
	{
		static uint64_t const type = CT_HASH("RenderModelLoadingDesc");
		return type;
	}

	virtual bool StateLess() const
	{
		return false;
	}
	virtual std::shared_ptr<void> CreateResource() 
	{
		auto nOff = model_desc_.strName.find_first_of("/");
		auto strName = model_desc_.strName.substr(0, nOff);
		RenderModelPtr pModel = model_desc_.CreateModelFactoryFunc(L"Model", strName);
		*model_desc_.pModel = pModel;
		return pModel;
	}

	virtual void SubThreadStage()
	{
		std::lock_guard<std::mutex> lock(main_thread_stage_mutex_);
		const RenderModelPtr& model = *model_desc_.pModel;

		std::vector<std::string> mesh_names;
		std::vector<uint32_t> mtl_ids;
		std::vector<uint32_t> mesh_lods;
		std::vector<zbVertex4D> VerticeVec;
		std::vector<int3> Int3diceVec;
		LoadModel(model_desc_.strName, model_desc_.pModelData->MaterialVec, mtl_ids, mesh_lods,
			mesh_names, model_desc_.pModelData->VerticeVec, Int3diceVec);

		model_desc_.pModelData->MeshVec.resize(mesh_names.size());
		uint32_t mesh_lod_index = 0;
		for (size_t mesh_index = 0; mesh_index < mesh_names.size(); ++mesh_index)
		{
			model_desc_.pModelData->MeshVec[mesh_index].strName = mesh_names[mesh_index];
			model_desc_.pModelData->MeshVec[mesh_index].nMtlId = mtl_ids[mesh_index];
			model_desc_.pModelData->MeshVec[mesh_index].nLods = mesh_lods[mesh_index];

			uint32_t const lods = model_desc_.pModelData->MeshVec[mesh_index].nLods;
			//model_desc_.pModelData->MeshVec[mesh_index].Int3diceVec.resize(lods);
			//memcpy(&model_desc_.pModelData->MeshVec[mesh_index].Int3diceVec[0], &Int3diceVec[mesh_lod_index], lods * sizeof(int3));
			mesh_lod_index += lods;
		}
	}

	virtual void MainThreadStage()
	{
		std::lock_guard<std::mutex> lock(main_thread_stage_mutex_);
		this->MainThreadStageNoLock();
	}

	virtual bool HasSubThreadStage() const
	{
		return true;
	}

	virtual bool Match(ResLoadingDesc const & rhs) const
	{
		if (this->Type() == rhs.Type())
		{
			ModelLoadingDesc const & rmld = static_cast<ModelLoadingDesc const &>(rhs);
			return (model_desc_.strName == rmld.model_desc_.strName)
				&& (model_desc_.nAttr == rmld.model_desc_.nAttr);
		}
		return false;
	}

	virtual void CopyDataFrom(ResLoadingDesc const & rhs)
	{
		BOOST_ASSERT(this->Type() == rhs.Type());

		ModelLoadingDesc const & rmld = static_cast<const ModelLoadingDesc&>(rhs);
		model_desc_.strName = rmld.model_desc_.strName;
		model_desc_.nAttr = rmld.model_desc_.nAttr;
		model_desc_.pModelData = rmld.model_desc_.pModelData;
		model_desc_.pModel = rmld.model_desc_.pModel;
	}

	virtual std::shared_ptr<void> CloneResourceFrom(std::shared_ptr<void> const & resource)
	{
		RenderModelPtr rhs_model = std::static_pointer_cast<RenderModel>(resource);
		RenderModelPtr model = model_desc_.CreateModelFactoryFunc(rhs_model->GetModleName(), rhs_model->GetConfigID());

		model->SetMaterialNum(rhs_model->GetMaterialNum());
		for (uint32_t mtl_index = 0; mtl_index < model->GetMaterialNum(); ++mtl_index)
		{
			RenderMaterialPtr mtl = MakeSharedPtr<RenderMaterial>();
			*mtl = *rhs_model->GetMaterial(mtl_index);
			model->GetMaterial(mtl_index) = mtl;
		}

		if (rhs_model->GetSubVisBaseNum() > 0)
		{
			std::vector<StaticMeshPtr> meshes(rhs_model->GetSubVisBaseNum());
			for (uint32_t mesh_index = 0; mesh_index < rhs_model->GetSubVisBaseNum(); ++mesh_index)
			{
				StaticMeshPtr rhs_mesh = checked_pointer_cast<StaticMesh>(rhs_model->SubVisBase(mesh_index));

				meshes[mesh_index] = model_desc_.CreateMeshFactoryFunc(model, rhs_mesh->GetName());
				StaticMeshPtr& mesh = meshes[mesh_index];
				mesh->SetMaterialID(rhs_mesh->GetMaterialID());
			}

			model->AssignSubVisbase(meshes.begin(), meshes.end());
		}

		for (uint32_t i = 0; i < model->GetSubVisBaseNum(); ++i)
		{
			checked_pointer_cast<StaticMesh>(model->SubVisBase(i))->DoBuildMeshInfo();
		}
		return std::static_pointer_cast<void>(model);
	}

	virtual std::shared_ptr<void> Resource() const override
	{
		return *model_desc_.pModel;
	}
private:
	ModelDesc model_desc_;
	std::mutex main_thread_stage_mutex_;

private:
	void FillModel()
	{
		RenderModelPtr const & model = *model_desc_.pModel;
		model->SetMaterialNum(model_desc_.pModelData->MaterialVec.size());
		for (uint32_t mtl_index = 0; mtl_index < model_desc_.pModelData->MaterialVec.size(); ++mtl_index)
		{
			model->GetMaterial(mtl_index) = model_desc_.pModelData->MaterialVec[mtl_index];
		}

		auto vb = DxGraphDevice::Instance()->MakeCreationVertexBuffer(model_desc_.pModelData->VerticeVec);
		std::vector<StaticMeshPtr> meshes(model_desc_.pModelData->MeshVec.size());
		for (uint32_t mesh_index = 0; mesh_index < model_desc_.pModelData->MeshVec.size(); ++mesh_index)
		{
			std::wstring wname = UtilString::as_wstring(model_desc_.pModelData->MeshVec[mesh_index].strName.c_str());

			meshes[mesh_index] = model_desc_.CreateMeshFactoryFunc(model, wname);
			StaticMeshPtr& mesh = meshes[mesh_index];
			mesh->SetMaterialID(model_desc_.pModelData->MeshVec[mesh_index].nMtlId);
			uint32_t nLod = model_desc_.pModelData->MeshVec[mesh_index].nLods;
			mesh->AddVertexStream(vb);
			mesh->AddIndexStream(nLod, model_desc_.pModelData->MeshVec[mesh_index].Int3diceVec);
		}

		model->AssignSubVisbase(meshes.begin(), meshes.end());
	}

	void MainThreadStageNoLock()
	{
		const RenderModelPtr& model = *model_desc_.pModel;

		if (nullptr != model)
		{
			this->FillModel();
		}
	}
};

RenderModel::RenderModel(Context* pContext)
	: Renderable(pContext)
{

}

RenderModel::RenderModel(Context* pContext, const std::wstring& name, std::string strConfigID)
	:Renderable(pContext),m_strTypeName(name),m_strConfigID(strConfigID)
{

}

RenderModel::~RenderModel()
{

}

void RenderModel::OnRenderBegin()
{
	for (auto const & mesh : m_SubVisbase)
	{
		mesh->OnRenderBegin();
	}
}

void RenderModel::OnRenderEnd()
{
	for (auto const & mesh : m_SubVisbase)
	{
		mesh->OnRenderEnd();
	}
}

void RenderModel::SetMaterialNum(int nCount)
{
	m_MaterialPtrVec.resize(nCount);
}

size_t RenderModel::GetMaterialNum() const
{
	return m_MaterialPtrVec.size();
}

RenderMaterialPtr& RenderModel::GetMaterial(size_t nIndex)
{
	BOOST_ASSERT(nIndex <= -1);
	BOOST_ASSERT(nIndex < GetMaterialNum());

	return m_MaterialPtrVec[nIndex];
}

void RenderModel::LoadMeshTexture()
{
	for (auto mesh : m_SubVisbase)
	{
		std::static_pointer_cast<StaticMesh>(mesh)->DoBuildMeshInfo();
	}
}

StaticMesh::StaticMesh(Context* pContext, const RenderModelPtr& model, const std::wstring& name)
	:Renderable(pContext), m_strName(name), m_pModel(model)
{
	m_LayoutPtr = MakeSharedPtr<RenderLayout>();
}

StaticMesh::StaticMesh(Context* pContext)
	:Renderable(pContext)
{
	m_LayoutPtr = MakeSharedPtr<RenderLayout>();
}

void StaticMesh::RegisterObject(Context* pContext)
{
	pContext->RegisterFactory<StaticMesh>();
}

StaticMesh::~StaticMesh()
{
}

void StaticMesh::DoBuildMeshInfo()
{
	RenderModelPtr model = m_pModel.lock();
	m_Mtl = model->GetMaterial(this->GetMaterialID());

	for (size_t i = 0; i < RenderMaterial::TS_TypeCount; ++i)
	{
		if (!m_Mtl->m_TexNames[i].empty())
		{
			auto strPathFile = model->GetConfigID() + "/";
			strPathFile += m_Mtl->m_TexNames[i];
			if (!ResLoader::Instance()->Locate(strPathFile).empty())
			{
				m_Textures[i] = SyncLoadTexture(strPathFile, 0);
			}
		}
	}
}

void StaticMesh::AddVertexStream(const VertexBuffer& vb)
{
	m_LayoutPtr->BindVertexStream(vb);
}

void StaticMesh::AddIndexStream(uint32_t nLod, const std::vector<int3>& Indices)
{
	m_LayoutPtr->BindIndexStream(nLod, Indices);
}

void LoadModel(const std::string strFineName, 
	std::vector<RenderMaterialPtr>& mtls,
	std::vector<uint32_t>& mtl_ids,
	std::vector<uint32_t>& mesh_lods,
	std::vector<std::string>& mesh_names,
	std::vector<zbVertex4D>& VerticeVec,
	std::vector<int3>& Int3diceVec)
{
 	tinyobj::attrib_t attrib;
 	std::vector<tinyobj::shape_t> shapes;
 	std::vector<tinyobj::material_t> materials;
 	std::string err;
 	auto lzma_file = ResLoader::Instance()->Open(strFineName);
	std::filesystem::path last_fxml_path(lzma_file->ResName());
	std::filesystem::path last_fxml_directory = last_fxml_path.parent_path();
	tinyobj::MaterialFileReader readMatFn(last_fxml_directory.string() + "/");
 	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, &(lzma_file->input_stream()), &readMatFn);

	mtls.resize(materials.size());
	for (uint32_t mtl_index = 0; mtl_index < static_cast<uint32_t>(materials.size()); ++mtl_index)
	{
		RenderMaterialPtr mtl = MakeSharedPtr<RenderMaterial>();
		mtls[mtl_index] = mtl;

		mtl->m_strName = materials[mtl_index].name;

		mtl->m_f4Albedo.x() = materials[mtl_index].ambient[0];
		mtl->m_f4Albedo.y() = materials[mtl_index].ambient[1];
		mtl->m_f4Albedo.z() = materials[mtl_index].ambient[2];
		mtl->m_f4Albedo.w() = materials[mtl_index].ambient[3];

		mtl->m_fMetalness = materials[mtl_index].metallic;

		mtl->m_fGlossiness = materials[mtl_index].sheen;

		mtl->m_f3Emissive.x() = materials[mtl_index].emission[0];
		mtl->m_f3Emissive.y() = materials[mtl_index].emission[1];
		mtl->m_f3Emissive.z() = materials[mtl_index].emission[2];

		if ('\0' != materials[mtl_index].ambient_texname[0])
		{
			mtl->m_TexNames[RenderMaterial::TS_Albedo] = materials[mtl_index].ambient_texname;
		}
		if ('\0' != materials[mtl_index].metallic_texname[0])
		{
			mtl->m_TexNames[RenderMaterial::TS_Metalness] = materials[mtl_index].metallic_texname;
		}
		if ('\0' != materials[mtl_index].sheen_texname[0])
		{
			mtl->m_TexNames[RenderMaterial::TS_Glossiness] = materials[mtl_index].sheen_texname;
		}
		if ('\0' != materials[mtl_index].emissive_texname[0])
		{
			mtl->m_TexNames[RenderMaterial::TS_Emissive] = materials[mtl_index].emissive_texname;
		}
		if ('\0' != materials[mtl_index].normal_texname[0])
		{
			mtl->m_TexNames[RenderMaterial::TS_Normal] = materials[mtl_index].normal_texname;
		}
		if ('\0' != materials[mtl_index].specular_highlight_texname[0])
		{
			mtl->m_TexNames[RenderMaterial::TS_Height] = materials[mtl_index].specular_highlight_texname;
		}
	}
 	// Loop over shapes
 	for (uint32_t s = 0; s < static_cast<uint32_t>(shapes.size()); s++)
 	{
		mesh_lods.push_back(shapes[s].mesh.num_face_vertices.size());
		mtl_ids.push_back(s);
		mesh_names.push_back(shapes[s].name);
 		// Loop over faces(polygon)
 		size_t index_offset = 0;
 		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
 		{
 			size_t fv = shapes[s].mesh.num_face_vertices[f];
 			// Loop over vertices in the face.
 			for (size_t v = 0; v < fv; v++) 
 			{
 				// access to vertex
 				zbVertex4D tmp;
				memset(&tmp, 0, sizeof(zbVertex4D));
 				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
 				tmp.v.x() = attrib.vertices[3 * idx.vertex_index + 0];
 				tmp.v.y() = attrib.vertices[3 * idx.vertex_index + 1];
 				tmp.v.z() = attrib.vertices[3 * idx.vertex_index + 2];
 				if (-1 != idx.normal_index)
 				{
 					tmp.n.x() = attrib.normals[3 * idx.normal_index + 0];
 					tmp.n.y() = attrib.normals[3 * idx.normal_index + 1];
 					tmp.n.z() = attrib.normals[3 * idx.normal_index + 2];
 				}
 				tmp.t.x() = attrib.texcoords[2 * idx.texcoord_index + 0];
 				tmp.t.y() = attrib.texcoords[2 * idx.texcoord_index + 1];
 
 				VerticeVec.push_back(tmp);
 			}
 			index_offset += fv;
 			// per-face material
 			shapes[s].mesh.material_ids[f];
 		}
 	}


}

RenderModelPtr SyncLoadModel(const std::string& strFileName, uint32_t nAttr,
	std::function<RenderModelPtr(const std::wstring &, const std::string&)> CreateModelFactoryFunc /*= CreateModelFactory<RenderModel>()*/,
	std::function<StaticMeshPtr(RenderModelPtr const &, const std::wstring&)> CreateMeshFactoryFunc /*= CreateMeshFactory<StaticMesh>()*/) 
{
	BOOST_ASSERT(CreateModelFactoryFunc);
	BOOST_ASSERT(CreateMeshFactoryFunc);

	return ResLoader::Instance()->SyncQueryT<RenderModel>(
		MakeSharedPtr<ModelLoadingDesc>(strFileName, nAttr, CreateModelFactoryFunc, CreateMeshFactoryFunc));
}

RenderModelPtr ASyncLoadModel(const std::string& strFileName, uint32_t nAttr, 
	std::function<RenderModelPtr(const std::wstring&, const std::string&)> CreateModelFactoryFunc /*= CreateModelFactory<RenderModel>()*/,
	std::function<StaticMeshPtr(const RenderModelPtr&, const std::wstring&)> CreateMeshFactoryFunc /*= CreateMeshFactory<StaticMesh>()*/)
{
	BOOST_ASSERT(CreateModelFactoryFunc);
	BOOST_ASSERT(CreateMeshFactoryFunc);

	return ResLoader::Instance()->ASyncQueryT<RenderModel>(
		MakeSharedPtr<ModelLoadingDesc>(strFileName, nAttr, CreateModelFactoryFunc, CreateMeshFactoryFunc));
}
