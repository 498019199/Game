#pragma once
#include <render/Renderable.h>
#include <render/RenderLayout.h>
#include <render/RenderMaterial.h>
#include <world/SceneNode.h>

#include <string_view>
#include <functional>

namespace RenderWorker
{
class RenderModel;
using RenderModelPtr = std::shared_ptr<RenderModel>;
class StaticMesh;
using StaticMeshPtr = std::shared_ptr<StaticMesh>;

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

ZENGINE_CORE_API void AddToSceneHelper(SceneNode& node, RenderModel& model);
ZENGINE_CORE_API void AddToSceneRootHelper(RenderModel& model);

class ZENGINE_CORE_API StaticMesh:public Renderable
{
    ZENGINE_NONCOPYABLE(StaticMesh);
public:
    explicit StaticMesh(std::wstring_view name);

    void BuildMeshInfo(const RenderModel& model);

    virtual void Technique(const RenderEffectPtr& effect, RenderTechnique* tech)
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

    virtual bool IsSkinned() const
    {
        return false;
    }

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

class ZENGINE_CORE_API JointComponent : public SceneComponent
{
public:
    NANO_RTTI_REGISTER_RUNTIME_CLASS(SceneComponent)

    SceneComponentPtr Clone() const override;

    void BindParams(quater const& real, quater const& dual, float scale);

    quater const& BindReal() const
    {
        return bind_real_;
    }
    quater const& BindDual() const
    {
        return bind_dual_;
    }
    float BindScale() const
    {
        return bind_scale_;
    }

    void InverseOriginParams(quater const& real, quater const& dual, float scale);

    quater const& InverseOriginReal() const
    {
        return inverse_origin_real_;
    }
    quater const& InverseOriginDual() const
    {
        return inverse_origin_dual_;
    }
    float InverseOriginScale() const
    {
        return inverse_origin_scale_;
    }

    void InitInverseOriginParams();

private:
    quater bind_real_;
    quater bind_dual_;
    float bind_scale_;

    quater inverse_origin_real_;
    quater inverse_origin_dual_;
    float inverse_origin_scale_;
};
using JointComponentPtr = std::shared_ptr<JointComponent>;

struct ZENGINE_CORE_API KeyFrameSet
{
    std::vector<uint32_t> frame_id;
    std::vector<quater> bind_real;
    std::vector<quater> bind_dual;
    std::vector<float> bind_scale;

    std::tuple<quater, quater, float> Frame(float frame) const;
};

struct ZENGINE_CORE_API AABBKeyFrameSet
{
    std::vector<uint32_t> frame_id;
    std::vector<AABBox> bb;

    AABBox Frame(float frame) const;
};

struct ZENGINE_CORE_API Animation
{
    std::string name;
    uint32_t start_frame;
    uint32_t end_frame;
};

class ZENGINE_CORE_API SkinnedModel : public RenderModel
{
    ZENGINE_NONCOPYABLE(SkinnedModel);
public:
    explicit SkinnedModel(const SceneNodePtr& root_node);
    SkinnedModel(std::wstring_view name, uint32_t node_attrib);
 
    bool IsSkinned() const override
    {
        return true;
    }

    void CloneDataFrom(RenderModel const & source,
        std::function<StaticMeshPtr(std::wstring_view)> const & CreateMeshFactoryFunc = CreateMeshFactory<StaticMesh>) override;

    JointComponentPtr& GetJoint(uint32_t index)
    {
        return joints_[index];
    }
    JointComponentPtr const& GetJoint(uint32_t index) const
    {
        return joints_[index];
    }
    uint32_t NumJoints() const
    {
        return static_cast<uint32_t>(joints_.size());
    }

    template <typename ForwardIterator>
    void AssignJoints(ForwardIterator first, ForwardIterator last)
    {
        joints_.assign(first, last);
        this->UpdateBinds();
    }

    void AttachKeyFrameSets(std::shared_ptr<std::vector<KeyFrameSet>> const & kf)
    {
        key_frame_sets_ = kf;
    }
    std::shared_ptr<std::vector<KeyFrameSet>> const & GetKeyFrameSets() const
    {
        return key_frame_sets_;
    }

    uint32_t NumFrames() const
    {
        return num_frames_;
    }
    void NumFrames(uint32_t nf)
    {
        num_frames_ = nf;
    }
    uint32_t FrameRate() const
    {
        return frame_rate_;
    }
    void FrameRate(uint32_t fr)
    {
        frame_rate_ = fr;
    }

    float GetFrame() const;
    void SetFrame(float frame);

    void RebindJoints();
    void UnbindJoints();

    AABBox FramePosBound(uint32_t frame) const;

    void AttachAnimations(std::shared_ptr<std::vector<Animation>> const & animations);
    std::shared_ptr<std::vector<Animation>> const & GetAnimations() const
    {
        return animations_;
    }
    uint32_t NumAnimations() const;
    void GetAnimation(uint32_t index, std::string& name, uint32_t& start_frame, uint32_t& end_frame);

protected:
    void BuildBones(float frame);
    void UpdateBinds();
    void SetToEffect();

protected:
    std::vector<JointComponentPtr> joints_;
    std::vector<float4> bind_reals_;
    std::vector<float4> bind_duals_;

    std::shared_ptr<std::vector<KeyFrameSet>> key_frame_sets_;
    float last_frame_;

    uint32_t num_frames_;
    uint32_t frame_rate_;

	std::shared_ptr<std::vector<Animation>> animations_;
};
using SkinnedModelPtr = std::shared_ptr<SkinnedModel>;

class ZENGINE_CORE_API SkinnedMesh : public StaticMesh
{
public:
    explicit SkinnedMesh(std::wstring_view name);

    AABBox FramePosBound(uint32_t frame) const;
    void AttachFramePosBounds(std::shared_ptr<AABBKeyFrameSet> const & frame_pos_aabbs);
    std::shared_ptr<AABBKeyFrameSet> const & GetFramePosBounds() const
    {
        return frame_pos_aabbs_;
    }

private:
    std::shared_ptr<AABBKeyFrameSet> frame_pos_aabbs_;
};
using SkinnedMeshPtr = std::shared_ptr<SkinnedMesh>;

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