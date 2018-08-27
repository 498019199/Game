// 2018年8月8日 zhangbei 模型加载
#ifndef _OBJ_MODEL_H_
#define _OBJ_MODEL_H_
#pragma once
#include "../Container/RenderVariable.h"
#include "../Render/Renderable.h"
#include "../Render/RenderLayout.h"
#include <functional>
class RenderModel :public Renderable
{
public:
	STX_ENTITY(RenderModel, Renderable);

	explicit RenderModel(Context* pContext);

	RenderModel(Context* pContext, std::wstring const & name, std::string strConfigID);

	virtual ~RenderModel();

	virtual void OnRenderBegin() override;

	virtual void OnRenderEnd() override;

	std::wstring GetModleName() const { return m_strTypeName; }
	std::string GetConfigID() { return m_strConfigID; }

	void SetMaterialNum(int nCount);
	size_t GetMaterialNum() const;
	MaterialPtr& GetMaterial(size_t nIndex);

	RenderLayoutPtr GetRenderLayout() const override { return m_LayoutPtr; }

	// 加载网格材质图片
	virtual void LoadMeshTexture();
protected:
private:
	// 类型名字
	std::wstring m_strTypeName;
	// 名字
	std::string m_strConfigID;
	// 材质列表
	std::vector<MaterialPtr> m_MaterialPtrVec;
	RenderLayoutPtr m_LayoutPtr;
};

class StaticMesh :public Renderable
{
public:
	STX_ENTITY(StaticMesh, Renderable);

	explicit StaticMesh(Context* pContext);

	StaticMesh(Context* pContext, const RenderModelPtr& model, const std::wstring& name);

	static void RegisterObject(Context* pContext);

	virtual ~StaticMesh();

	// 节点名字
	std::wstring GetName() { return m_strName; }

	// 节点编号
	int32_t GetMaterialID() const { return m_nMtlID; }
	void SetMaterialID(int32_t mid) { m_nMtlID = mid; }

	virtual void DoBuildMeshInfo();

	void AddVertexStream(const VertexBuffer& vb);
	void AddIndexStream(uint32_t nLod, const std::vector<int3>& Indices);
	RenderLayoutPtr GetRenderLayout() const override { return m_LayoutPtr; }
protected:
	int32_t m_nMtlID;
	std::wstring m_strName;
	std::weak_ptr<RenderModel> m_pModel;
	RenderLayoutPtr m_LayoutPtr;
};

template <typename T>
struct CreateMeshFactory
{
	StaticMeshPtr operator()(RenderModelPtr const & model, std::wstring const & name)
	{
		return Context::Instance()->CreateObjectArgs<StaticMesh>(Context::Instance(), model, name);
		return nullptr;
	}
};

template <typename T>
struct CreateModelFactory
{
	RenderModelPtr operator()(const std::wstring& name, const std::string& strConfigID)
	{
		return MakeSharedPtr<T>(Context::Instance(), name, strConfigID);
		return nullptr;
	}
};

template <typename T>
struct CreateSceneObjectFactory
{
	SceneObjectPtr operator()(const RenderModelPtr& model)
	{
		return MakeSharedPtr<T>(model, SceneObject::SOA_Cullable);
	}
};

void LoadModel(const std::string strFineName,
	std::vector<MaterialPtr>& mtls,
	std::vector<uint32_t>& mtl_ids,
	std::vector<uint32_t>& mesh_lods,
	std::vector<std::string>& mesh_names,
	std::vector<zbVertex4D>& VerticeVec,
	std::vector<int3>& Int3diceVec);

RenderModelPtr SyncLoadModel(const std::string& strFileName, uint32_t nAttr,
	std::function<RenderModelPtr(const std::wstring &, const std::string&)> CreateModelFactoryFunc = CreateModelFactory<RenderModel>(),
	std::function<StaticMeshPtr(RenderModelPtr const &, const std::wstring&)> CreateMeshFactoryFunc = CreateMeshFactory<StaticMesh>());

RenderModelPtr ASyncLoadModel(const std::string& strFileName, uint32_t nAttr,
	std::function<RenderModelPtr(const std::wstring&, const std::string&)> CreateModelFactoryFunc = CreateModelFactory<RenderModel>(),
	std::function<StaticMeshPtr(const RenderModelPtr&, const std::wstring&)> CreateMeshFactoryFunc = CreateMeshFactory<StaticMesh>());
#endif//_OBJ_MODEL_H_