#pragma once
#include <render/Renderable.h>
#include <render/RenderLayout.h>
#include <render/RenderMaterial.h>
#include <string_view>
#include <functional>

namespace RenderWorker
{
class StaticMesh;
using StaticMeshPtr = std::shared_ptr<StaticMesh>;
class RenderModel;
using RenderModelPtr = std::shared_ptr<RenderModel>;

template <typename T>
inline StaticMeshPtr CreateMeshFactory(std::wstring_view name)
{
    return MakeSharedPtr<T>(name);
}

template <typename T>
inline RenderModelPtr CreateModelFactory(std::wstring_view name, uint32_t node_attrib)
{
    return MakeSharedPtr<T>(name, node_attrib);
}


class ZENGINE_CORE_API StaticMesh:public Renderable
{
    ZENGINE_NONCOPYABLE(StaticMesh);
public:
    explicit StaticMesh(std::wstring_view name);

    void BuildMeshInfo(const RenderModel& model);

    virtual void Technique(RenderEffectPtr const & effect, RenderTechnique* tech)
    {
		effect_ = effect;
		technique_ = tech;
	}

    void NumLods(uint32_t lods) override;
    using Renderable::NumLods;

    virtual void PosBound(const AABBox& aabb);
    using Renderable::PosBound;
    virtual void TexcoordBound(const AABBox& aabb);
    using Renderable::TexcoordBound;

    void NumVertices(uint32_t lod, uint32_t n)
    {
        rls_[lod]->NumVertices(n);
    }
    uint32_t NumVertices(uint32_t lod) const
    {
        return rls_[lod]->NumVertices();
    }

    void NumIndices(uint32_t lod, uint32_t n)
    {
        rls_[lod]->NumIndices(n);
    }
    uint32_t NumIndices(uint32_t lod) const
    {
        return rls_[lod]->NumIndices();
    }

    void AddVertexStream(uint32_t lod, const void * buf, uint32_t size, const VertexElement & ve, uint32_t access_hint);
    void AddVertexStream(uint32_t lod, const GraphicsBufferPtr & buffer, const VertexElement & ve);
    void AddIndexStream(uint32_t lod, const void * buf, uint32_t size, ElementFormat format, uint32_t access_hint);
    void AddIndexStream(uint32_t lod, const GraphicsBufferPtr & index_stream, ElementFormat format);

	void StartVertexLocation(uint32_t lod, uint32_t location)
	{
		rls_[lod]->StartVertexLocation(location);
	}
	uint32_t StartVertexLocation(uint32_t lod) const
	{
		return rls_[lod]->StartVertexLocation();
	}

	void StartIndexLocation(uint32_t lod, uint32_t location)
	{
		rls_[lod]->StartIndexLocation(location);
	}
	uint32_t StartIndexLocation(uint32_t lod) const
	{
		return rls_[lod]->StartIndexLocation();
	}

	// void StartInstanceLocation(uint32_t lod, uint32_t location)
	// {
	// 	rls_[lod]->StartInstanceLocation(location);
	// }
	// uint32_t StartInstanceLocation(uint32_t lod) const
	// {
	// 	return rls_[lod]->StartInstanceLocation();
	// }

	int32_t MaterialID() const
	{
		return mtl_id_;
	}
	void MaterialID(int32_t mid)
	{
		mtl_id_ = mid;
	}

	bool HWResourceReady() const override
	{
		return hw_res_ready_;
	}
protected:
	virtual void DoBuildMeshInfo(const RenderModel& model);

protected:
    int32_t mtl_id_;
    bool hw_res_ready_;
};

class ZENGINE_CORE_API RenderModel
{
    ZENGINE_NONCOPYABLE(RenderModel);
public:
    explicit RenderModel(const SceneNodePtr& root_node);
	RenderModel(std::wstring_view name, uint32_t node_attrib);
    virtual ~RenderModel() noexcept;

    const SceneNodePtr& RootNode() const
    {
        return root_node_;
    }

    void BuildModelInfo();

    uint32_t NumLods() const;
    void ActiveLod(int32_t lod);

    size_t NumMaterials() const
    {
        return materials_.size();
    }
    void NumMaterials(size_t i)
    {
        materials_.resize(i);
    }

    template <typename ForwardIterator>
    void AssignMeshes(ForwardIterator first, ForwardIterator last)
    {
        meshes_.assign(first, last);
    }
    RenderMaterialPtr& GetMaterial(int32_t i)
    {
        return materials_[i];
    }
    const RenderMaterialPtr& GetMaterial(int32_t i) const
    {
        return materials_[i];
    }

    const RenderablePtr& Mesh(size_t id) const
    {
        return meshes_[id];
    }
    uint32_t NumMeshes() const
    {
        return static_cast<uint32_t>(meshes_.size());
    }
    
    void ForEachMesh(std::function<void(Renderable&)> const & callback) const;

    bool HWResourceReady() const;
    
    RenderModelPtr Clone(
        const std::function<RenderModelPtr(std::wstring_view, uint32_t)>& CreateModelFactoryFunc = CreateModelFactory<RenderModel>,
        const std::function<StaticMeshPtr(std::wstring_view)>& CreateMeshFactoryFunc = CreateMeshFactory<StaticMesh>);
    virtual void CloneDataFrom(const RenderModel& source,
        const std::function<StaticMeshPtr(std::wstring_view)>& CreateMeshFactoryFunc = CreateMeshFactory<StaticMesh>);

protected:
    virtual void DoBuildModelInfo()
    {
    }

protected:
	SceneNodePtr root_node_;
    std::vector<RenderablePtr> meshes_;
    std::vector<RenderMaterialPtr> materials_;

    bool hw_res_ready_;
};

class ZENGINE_CORE_API SkinnedModel : public RenderModel
{
    ZENGINE_NONCOPYABLE(SkinnedModel);
public:
    explicit SkinnedModel();
    SkinnedModel(std::wstring_view name, uint32_t node_attrib);
 
};

ZENGINE_CORE_API RenderModelPtr SyncLoadModel(std::string_view model_name, uint32_t access_hint,
    uint32_t node_attrib,
    std::function<void(RenderModel&)> OnFinishLoading = nullptr,
    std::function<RenderModelPtr(std::wstring_view, uint32_t)> CreateModelFactoryFunc = CreateModelFactory<RenderModel>,
    std::function<StaticMeshPtr(std::wstring_view)> CreateMeshFactoryFunc = CreateMeshFactory<StaticMesh>);
ZENGINE_CORE_API RenderModelPtr ASyncLoadModel(std::string_view model_name, uint32_t access_hint,
    uint32_t node_attrib,
    std::function<void(RenderModel&)> OnFinishLoading = nullptr,
    std::function<RenderModelPtr(std::wstring_view, uint32_t)> CreateModelFactoryFunc = CreateModelFactory<RenderModel>,
    std::function<StaticMeshPtr(std::wstring_view)> CreateMeshFactoryFunc = CreateMeshFactory<StaticMesh>);
ZENGINE_CORE_API RenderModelPtr LoadSoftwareModel(std::string_view model_name);
ZENGINE_CORE_API void SaveModel(const RenderModel & model, std::string_view model_name);
}