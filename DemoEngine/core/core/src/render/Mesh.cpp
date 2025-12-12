#include <render/Mesh.h>
#include <common/Util.h>
#include <base/ResLoader.h>
#include <base/DevHelper.h>
#include <base/LZMACodec.h>

#include <render/RenderFactory.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <cstring>

namespace
{
	using namespace RenderWorker;
	using namespace CommonWorker;
	uint32_t const MODEL_BIN_VERSION = 19;

    class RenderModelLoadingDesc : public ResLoadingDesc
    {
	private:
		struct ModelDesc
		{
			std::string res_name;
			uint32_t access_hint;
			uint32_t node_attrib;
			std::function<void(RenderModel&)> OnFinishLoading;
			std::function<RenderModelPtr(std::wstring_view, uint32_t)> CreateModelFactoryFunc;
			std::function<StaticMeshPtr(std::wstring_view)> CreateMeshFactoryFunc;

			RenderModelPtr sw_model;
			std::shared_ptr<RenderModelPtr> model;
		};

	public:
		RenderModelLoadingDesc(std::string_view res_name, uint32_t access_hint, uint32_t node_attrib,
			std::function<void(RenderModel&)> OnFinishLoading,
			std::function<RenderModelPtr(std::wstring_view, uint32_t)> CreateModelFactoryFunc,
			std::function<StaticMeshPtr(std::wstring_view)> CreateMeshFactoryFunc)
		{
			model_desc_.res_name = std::string(res_name);
			model_desc_.access_hint = access_hint;
			model_desc_.node_attrib = node_attrib;
			model_desc_.OnFinishLoading = OnFinishLoading;
			model_desc_.CreateModelFactoryFunc = CreateModelFactoryFunc;
			model_desc_.CreateMeshFactoryFunc = CreateMeshFactoryFunc;
			model_desc_.model = MakeSharedPtr<RenderModelPtr>();

			this->AddsSubPath();
		}

		uint64_t Type() const override
		{
			return CtHash("RenderModelLoadingDesc");
		}

		bool StateLess() const override
		{
			return false;
		}

		std::shared_ptr<void> CreateResource() override
		{
			RenderModelPtr model = model_desc_.CreateModelFactoryFunc(L"Model", model_desc_.node_attrib);
			*model_desc_.model = model;
			return model;
		}

		void SubThreadStage() override
		{
			std::lock_guard<std::mutex> lock(main_thread_stage_mutex_);

			RenderModelPtr const & model = *model_desc_.model;
			if (model && model->HWResourceReady())
			{
				return;
			}

			model_desc_.sw_model = LoadSoftwareModel(model_desc_.res_name);

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();
			if (caps.multithread_res_creating_support)
			{
				this->MainThreadStageNoLock();
			}
		}

		void MainThreadStage() override
		{
			std::lock_guard<std::mutex> lock(main_thread_stage_mutex_);
			this->MainThreadStageNoLock();
		}

		bool HasSubThreadStage() const override
		{
			return true;
		}

		bool Match([[maybe_unused]] ResLoadingDesc const & rhs) const override
		{
			return false;
		}

		void CopyDataFrom(ResLoadingDesc const & rhs) override
		{
			COMMON_ASSERT(this->Type() == rhs.Type());

			RenderModelLoadingDesc const & rmld = static_cast<RenderModelLoadingDesc const &>(rhs);
			model_desc_.res_name = rmld.model_desc_.res_name;
			model_desc_.access_hint = rmld.model_desc_.access_hint;
			model_desc_.sw_model = rmld.model_desc_.sw_model;
			model_desc_.model = rmld.model_desc_.model;
		}

		std::shared_ptr<void> CloneResourceFrom(std::shared_ptr<void> const & resource) override
		{
			auto rhs_model = std::static_pointer_cast<RenderModel>(resource);
			auto model = model_desc_.CreateModelFactoryFunc(rhs_model->RootNode()->Name(), rhs_model->RootNode()->Attrib());
			model->CloneDataFrom(*rhs_model, model_desc_.CreateMeshFactoryFunc);

			model->BuildModelInfo();
			for (uint32_t i = 0; i < model->NumMeshes(); ++ i)
			{
				checked_pointer_cast<StaticMesh>(model->Mesh(i))->BuildMeshInfo(*model);
			}
			
			if (model_desc_.OnFinishLoading)
			{
				model_desc_.OnFinishLoading(*model);
			}

			return std::static_pointer_cast<void>(model);
		}

		std::shared_ptr<void> Resource() const override
		{
			return *model_desc_.model;
		}

	private:
		void FillModel()
		{
			auto const & model = *model_desc_.model;
			auto const & sw_model = *model_desc_.sw_model;

			model->CloneDataFrom(sw_model, model_desc_.CreateMeshFactoryFunc);

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			auto const & sw_rl = checked_pointer_cast<StaticMesh>(sw_model.Mesh(0))->GetRenderLayout(0);

			std::vector<GraphicsBufferPtr> merged_vbs(sw_rl.VertexStreamNum());
			for (uint32_t i = 0; i < sw_rl.VertexStreamNum(); ++ i)
			{
				merged_vbs[i] = rf.MakeDelayCreationVertexBuffer(BU_Static, model_desc_.access_hint, sw_rl.GetVertexStream(i)->Size());
			}
			auto merged_ib = rf.MakeDelayCreationIndexBuffer(BU_Static, model_desc_.access_hint, sw_rl.GetIndexStream()->Size());

			for (uint32_t mesh_index = 0; mesh_index < model->NumMeshes(); ++ mesh_index)
			{
				for (uint32_t lod = 0; lod < model->Mesh(mesh_index)->NumLods(); ++ lod)
				{
					auto& rl = model->Mesh(mesh_index)->GetRenderLayout(lod);
					for (uint32_t i = 0; i < sw_rl.VertexStreamNum(); ++ i)
					{
						rl.SetVertexStream(i, merged_vbs[i]);
					}
					rl.BindIndexStream(merged_ib, rl.IndexStreamFormat());
				}
			}
		}

		void AddsSubPath()
		{
			std::string sub_path;
			auto sub_path_loc = model_desc_.res_name.find_last_of('/');
			if (sub_path_loc != std::string::npos)
			{
				auto& res_loader = Context::Instance().ResLoaderInstance();
				sub_path = res_loader.Locate(model_desc_.res_name.substr(0, sub_path_loc));
				if (!sub_path.empty())
				{
					res_loader.AddPath(sub_path);
				}
			}
		}

		void MainThreadStageNoLock()
		{
			RenderModelPtr const & model = *model_desc_.model;
			if (!model || !model->HWResourceReady())
			{
				this->FillModel();

				auto const & sw_model = *model_desc_.sw_model;

				auto const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
				auto const & rl = checked_pointer_cast<StaticMesh>(model->Mesh(0))->GetRenderLayout();
				auto const & sw_rl = checked_pointer_cast<StaticMesh>(sw_model.Mesh(0))->GetRenderLayout();

				for (uint32_t i = 0; i < rl.VertexStreamNum(); ++ i)
				{
					GraphicsBuffer::Mapper mapper(*sw_rl.GetVertexStream(i), BA_Read_Only);

					uint32_t const num_vertices = sw_rl.GetVertexStream(i)->Size() / sizeof(uint32_t);

					void* ptr = mapper.Pointer<void>();
					auto ve = sw_rl.VertexStreamFormat(i)[0];

					std::vector<uint8_t> buff;
					if (!caps.VertexFormatSupport(ve.format))
					{
						buff.resize(sw_rl.GetVertexStream(i)->Size());
						ptr = buff.data();

						uint32_t const * src = mapper.Pointer<uint32_t>();
						uint32_t* dst = static_cast<uint32_t*>(ptr);

						if (ve.format == EF_A2BGR10)
						{
							ve.format = caps.BestMatchVertexFormat(MakeSpan({EF_ARGB8, EF_ABGR8}));

							if (ve.format == EF_ARGB8)
							{
								for (uint32_t j = 0; j < num_vertices; ++ j)
								{
									float x = ((src[j] >> 0) & 0x3FF) / 1023.0f;
									float y = ((src[j] >> 10) & 0x3FF) / 1023.0f;
									float z = ((src[j] >> 20) & 0x3FF) / 1023.0f;
									float w = ((src[j] >> 30) & 0x3) / 3.0f;

									dst[j] = (MathWorker::clamp(static_cast<uint32_t>(x * 255), 0U, 255U) << 16)
										| (MathWorker::clamp(static_cast<uint32_t>(y * 255), 0U, 255U) << 8)
										| (MathWorker::clamp(static_cast<uint32_t>(z * 255), 0U, 255U) << 0)
										| (MathWorker::clamp(static_cast<uint32_t>(w * 255), 0U, 255U) << 24);
								}
							}
							else
							{
								for (uint32_t j = 0; j < num_vertices; ++ j)
								{
									float x = ((src[j] >> 0) & 0x3FF) / 1023.0f;
									float y = ((src[j] >> 10) & 0x3FF) / 1023.0f;
									float z = ((src[j] >> 20) & 0x3FF) / 1023.0f;
									float w = ((src[j] >> 30) & 0x3) / 3.0f;

									dst[j] = (MathWorker::clamp(static_cast<uint32_t>(x * 255), 0U, 255U) << 0)
										| (MathWorker::clamp(static_cast<uint32_t>(y * 255), 0U, 255U) << 8)
										| (MathWorker::clamp(static_cast<uint32_t>(z * 255), 0U, 255U) << 16)
										| (MathWorker::clamp(static_cast<uint32_t>(w * 255), 0U, 255U) << 24);
								}
							}
						}
						else if (ve.format == EF_ARGB8)
						{
							COMMON_ASSERT(caps.VertexFormatSupport(EF_ABGR8));

							ve.format = EF_ABGR8;

							for (uint32_t j = 0; j < num_vertices; ++ j)
							{
								float x = ((src[j] >> 16) & 0xFF) / 255.0f;
								float y = ((src[j] >> 8) & 0xFF) / 255.0f;
								float z = ((src[j] >> 0) & 0xFF) / 255.0f;
								float w = ((src[j] >> 24) & 0xFF) / 255.0f;

								dst[j] = (MathWorker::clamp(static_cast<uint32_t>(x * 255), 0U, 255U) << 0)
									| (MathWorker::clamp(static_cast<uint32_t>(y * 255), 0U, 255U) << 8)
									| (MathWorker::clamp(static_cast<uint32_t>(z * 255), 0U, 255U) << 16)
									| (MathWorker::clamp(static_cast<uint32_t>(w * 255), 0U, 255U) << 24);
							}
						}
						else
						{
							ZENGINE_UNREACHABLE("Invalid vertex format");
						}
					}

					rl.GetVertexStream(i)->CreateHWResource(ptr);
				}
				{
					GraphicsBuffer::Mapper mapper(*sw_rl.GetIndexStream(), BA_Read_Only);
					rl.GetIndexStream()->CreateHWResource(mapper.Pointer<void>());
				}

				model->BuildModelInfo();
				for (uint32_t i = 0; i < model->NumMeshes(); ++ i)
				{
					checked_pointer_cast<StaticMesh>(model->Mesh(i))->BuildMeshInfo(*model);
				}

				model_desc_.sw_model.reset();

				if (model_desc_.OnFinishLoading)
				{
					model_desc_.OnFinishLoading(*model);
				}
			}
		}

	private:
		ModelDesc model_desc_;
		std::mutex main_thread_stage_mutex_;
    };

}


// write model file
namespace
{
	// void SaveModel(std::string const & jit_name, std::vector<RenderMaterialPtr> const & mtls,
	// 	std::vector<VertexElement> const & merged_ves, char all_is_index_16_bit,
	// 	std::vector<std::vector<uint8_t>> const & merged_buffs, std::vector<uint8_t> const & merged_indices,
	// 	std::vector<std::string> const & mesh_names, std::vector<int32_t> const & mtl_ids, std::vector<uint32_t> const & mesh_lods,
	// 	std::vector<AABBox> const & pos_bbs, std::vector<AABBox> const & tc_bbs,
	// 	std::vector<uint32_t> const & mesh_num_vertices, std::vector<uint32_t> const & mesh_base_vertices,
	// 	std::vector<uint32_t> const & mesh_num_indices, std::vector<uint32_t> const & mesh_base_indices,
	// 	std::vector<SceneNode const *> const & nodes, std::vector<Renderable const *> const & renderables,
	// 	std::vector<JointComponent const*> const & joints, std::shared_ptr<std::vector<Animation>> const & animations,
	// 	std::shared_ptr<std::vector<KeyFrameSet>> const & kfs, uint32_t num_frames, uint32_t frame_rate,
	// 	std::vector<std::shared_ptr<AABBKeyFrameSet>> const & frame_pos_bbs)
	// {
	// }
}

namespace RenderWorker
{
	StaticMesh::StaticMesh(std::wstring_view name)
		: Renderable(name), hw_res_ready_(false)
	{
	}

	void StaticMesh::BuildMeshInfo(const RenderModel& model)
	{
		this->DoBuildMeshInfo(model);
		hw_res_ready_ = true;
	}

	void StaticMesh::NumLods(uint32_t lods)
	{
		Renderable::NumLods(lods);

		for (auto& rl : rls_)
		{
			if (Context::Instance().RenderFactoryValid())
			{
				auto& rf = Context::Instance().RenderFactoryInstance();
				rl = rf.MakeRenderLayout();
			}
			else
			{
				rl = MakeSharedPtr<RenderLayout>();
			}
			rl->TopologyType(RenderLayout::TT_TriangleList);
		}
	}

	void StaticMesh::PosBound(const AABBox& aabb)
	{
		pos_aabb_ = aabb;
	}

	void StaticMesh::TexcoordBound(const AABBox& aabb)
	{
		tc_aabb_ = aabb;
	}

	void StaticMesh::DoBuildMeshInfo(const RenderModel& model)
	{
		bool is_skinned = false;
		for (uint32_t i = 0; !is_skinned && (i < rls_[0]->VertexStreamNum()); ++i)
		{
			auto const& vertex_stream_fmt = rls_[0]->VertexStreamFormat(i);
			for (auto const& vertex_elem : vertex_stream_fmt)
			{
				if ((vertex_elem.usage == VEU_BlendIndex) || (vertex_elem.usage == VEU_BlendWeight))
				{
					is_skinned = true;
					break;
				}
			}
		}

		this->IsSkinned(is_skinned);
		this->Material(model.GetMaterial(this->MaterialID()));
	}

	void StaticMesh::AddVertexStream(uint32_t lod, const void * buf, uint32_t size, const VertexElement & ve, uint32_t access_hint)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		GraphicsBufferPtr vb = rf.MakeVertexBuffer(BU_Static, access_hint, size, buf);
		this->AddVertexStream(lod, vb, ve);
	}

	void StaticMesh::AddVertexStream(uint32_t lod, const GraphicsBufferPtr & buffer, const VertexElement & ve)
	{
		rls_[lod]->BindVertexStream(buffer, ve);
	}

	void StaticMesh::AddIndexStream(uint32_t lod, const void * buf, uint32_t size, ElementFormat format, uint32_t access_hint)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static, access_hint, size, buf);
		this->AddIndexStream(lod, ib, format);
	}

	void StaticMesh::AddIndexStream(uint32_t lod, const GraphicsBufferPtr & index_stream, ElementFormat format)
	{
		rls_[lod]->BindIndexStream(index_stream, format);
	}

	RenderModel::RenderModel(const SceneNodePtr& root_node)
		: root_node_(root_node), hw_res_ready_(false)
	{
	}

	RenderModel::RenderModel(std::wstring_view name, uint32_t node_attrib)
		: RenderModel(MakeSharedPtr<SceneNode>(name, node_attrib))
	{
	}

	RenderModel::~RenderModel() noexcept = default;

	void RenderModel::BuildModelInfo()
	{
		this->DoBuildModelInfo();
		root_node_->UpdatePosBoundSubtree();
		hw_res_ready_ = true;
	}

	uint32_t RenderModel::NumLods() const
	{
		uint32_t max_lod = 0;
		this->ForEachMesh([&max_lod](Renderable& mesh)
			{
				max_lod = std::max(max_lod, mesh.NumLods());
			});
		return max_lod;
	}

	void RenderModel::ActiveLod(int32_t lod)
	{
		this->ForEachMesh([lod](Renderable& mesh)
			{
				mesh.ActiveLod(lod);
			});
	}


	void RenderModel::ForEachMesh(std::function<void(Renderable&)> const & callback) const
	{
		for (auto const & mesh : meshes_)
		{
			callback(*mesh);
		}
	}

	bool RenderModel::HWResourceReady() const
	{
		bool ready = hw_res_ready_;
		if (ready)
		{
			for (auto const & mesh : meshes_)
			{
				ready &= mesh->HWResourceReady();
				if (!ready)
				{
					break;
				}
			}
		}
		return ready;
	}

	RenderModelPtr RenderModel::Clone(const std::function<RenderModelPtr(std::wstring_view, uint32_t)>& CreateModelFactoryFunc,
        const std::function<StaticMeshPtr(std::wstring_view)>& CreateMeshFactoryFunc)
	{
		auto ret_model = CreateModelFactoryFunc(root_node_->Name(), root_node_->Attrib());
		ret_model->CloneDataFrom(*this, CreateMeshFactoryFunc);

		ret_model->BuildModelInfo();
		for (auto const & ret_mesh : ret_model->meshes_)
		{
			checked_pointer_cast<StaticMesh>(ret_mesh)->BuildMeshInfo(*ret_model);
		}

		return ret_model;
	}

	void RenderModel::CloneDataFrom(RenderModel const & source,
		std::function<StaticMeshPtr(std::wstring_view)> const & CreateMeshFactoryFunc)
	{
		this->NumMaterials(source.NumMaterials());
		for (uint32_t mtl_index = 0; mtl_index < source.NumMaterials(); ++ mtl_index)
		{
			auto& mtl = this->GetMaterial(mtl_index);
			mtl = source.GetMaterial(mtl_index)->Clone();
			mtl->LoadTextureSlots();
		}

		if (source.NumMeshes() > 0)
		{
			std::vector<StaticMeshPtr> meshes(source.NumMeshes());
			for (uint32_t mesh_index = 0; mesh_index < source.NumMeshes(); ++ mesh_index)
			{
				auto const& src_mesh = checked_cast<StaticMesh&>(*source.Mesh(mesh_index));

				meshes[mesh_index] = CreateMeshFactoryFunc(src_mesh.Name());
				auto& mesh = *meshes[mesh_index];

				mesh.MaterialID(src_mesh.MaterialID());
				mesh.NumLods(src_mesh.NumLods());
				mesh.PosBound(src_mesh.PosBound());
				mesh.TexcoordBound(src_mesh.TexcoordBound());

				for (uint32_t lod = 0; lod < src_mesh.NumLods(); ++ lod)
				{
					auto const & src_rl = src_mesh.GetRenderLayout(lod);

					for (uint32_t ve_index = 0; ve_index < src_rl.VertexStreamNum(); ++ ve_index)
					{
						mesh.AddVertexStream(lod, src_rl.GetVertexStream(ve_index), src_rl.VertexStreamFormat(ve_index)[0]);
					}
					mesh.AddIndexStream(lod, src_rl.GetIndexStream(), src_rl.IndexStreamFormat());

					mesh.NumVertices(lod, src_mesh.NumVertices(lod));
					mesh.NumIndices(lod, src_mesh.NumIndices(lod));
					mesh.StartVertexLocation(lod, src_mesh.StartVertexLocation(lod));
					mesh.StartIndexLocation(lod, src_mesh.StartIndexLocation(lod));
				}
			}

			this->AssignMeshes(meshes.begin(), meshes.end());
		}

		std::vector<SceneNode const *> source_nodes;
		std::vector<SceneNodePtr> new_nodes;
		source.RootNode()->Traverse([this, &source, &source_nodes, &new_nodes](SceneNode& node)
			{
				source_nodes.push_back(&node);

				SceneNodePtr new_node;
				if (new_nodes.empty())
				{
					new_node = root_node_;
				}
				else
				{
					new_node = MakeSharedPtr<SceneNode>(node.Name(), node.Attrib());

					for (size_t j = 0; j < source_nodes.size() - 1; ++ j)
					{
						if (node.Parent() == source_nodes[j])
						{
							new_nodes[j]->AddChild(new_node);
						}
					}
				}
				new_nodes.push_back(new_node);

				// for (uint32_t i = 0; i < node.NumComponents(); ++ i)
				// {
				// 	auto& component = *node.ComponentByIndex(i);
				// 	if (component.IsOfType<RenderableComponent>())
				// 	{
				// 		auto const* renderable = &NanoRtti::DynCast<RenderableComponent const*>(&component)->BoundRenderable();
				// 		for (uint32_t mesh_index = 0; mesh_index < source.NumMeshes(); ++mesh_index)
				// 		{
				// 			if (renderable == source.Mesh(mesh_index).get())
				// 			{
				// 				new_node->AddComponent(MakeSharedPtr<RenderableComponent>(this->Mesh(mesh_index)));
				// 				break;
				// 			}
				// 		}
				// 	}
				// 	else
				// 	{
				// 		new_node->AddComponent(component.Clone());
				// 	}
				// }
				// new_node->TransformToParent(node.TransformToParent());

				return true;
			});

		root_node_->UpdatePosBoundSubtree();
	}

    RenderModelPtr SyncLoadModel(std::string_view model_name, uint32_t access_hint, uint32_t node_attrib,
		std::function<void(RenderModel&)> OnFinishLoading,
		std::function<RenderModelPtr(std::wstring_view, uint32_t)> CreateModelFactoryFunc,
		std::function<StaticMeshPtr(std::wstring_view)> CreateMeshFactoryFunc)
	{
		COMMON_ASSERT(CreateModelFactoryFunc);
		COMMON_ASSERT(CreateMeshFactoryFunc);

		return Context::Instance().ResLoaderInstance().SyncQueryT<RenderModel>(MakeSharedPtr<RenderModelLoadingDesc>(model_name,
			access_hint, node_attrib, OnFinishLoading, CreateModelFactoryFunc, CreateMeshFactoryFunc));
	}

	RenderModelPtr ASyncLoadModel(std::string_view model_name, uint32_t access_hint, uint32_t node_attrib,
		std::function<void(RenderModel&)> OnFinishLoading,
		std::function<RenderModelPtr(std::wstring_view, uint32_t)> CreateModelFactoryFunc,
		std::function<StaticMeshPtr(std::wstring_view)> CreateMeshFactoryFunc)
	{
		COMMON_ASSERT(CreateModelFactoryFunc);
		COMMON_ASSERT(CreateMeshFactoryFunc);

		// Hacky code. During model creation, shaders are created, too. On devices without multirhead resource creating support, shaeder
		// creation will failed. It can be removed once we have async effect loading.
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		const RenderDeviceCaps & caps = rf.RenderEngineInstance().DeviceCaps();
		if (caps.multithread_res_creating_support)
		{
			return Context::Instance().ResLoaderInstance().ASyncQueryT<RenderModel>(MakeSharedPtr<RenderModelLoadingDesc>(model_name,
				access_hint, node_attrib, OnFinishLoading, CreateModelFactoryFunc, CreateMeshFactoryFunc));
		}
		else
		{
			return SyncLoadModel(model_name, access_hint, node_attrib, OnFinishLoading, CreateModelFactoryFunc, CreateMeshFactoryFunc);
		}
	}

	RenderModelPtr LoadSoftwareModel(std::string_view model_name)
	{
		char const * JIT_EXT_NAME = ".model_bin";

		auto& context = Context::Instance();
		auto& res_loader = context.ResLoaderInstance();

		std::string runtime_name(model_name);
		if (std::filesystem::path(runtime_name).extension() != JIT_EXT_NAME)
		{
			std::string const metadata_name = runtime_name + ".kmeta";
			runtime_name += JIT_EXT_NAME;

			bool jit = false;
			if (res_loader.Locate(runtime_name).empty())
			{
				jit = true;
			}
			else
			{
				ResIdentifierPtr runtime_file = res_loader.Open(runtime_name);
				uint32_t fourcc;
				runtime_file->read(&fourcc, sizeof(fourcc));
				fourcc = LE2Native(fourcc);
				uint32_t ver;
				runtime_file->read(&ver, sizeof(ver));
				ver = LE2Native(ver);
				if ((fourcc != MakeFourCC<'K', 'L', 'M', ' '>::value) || (ver != MODEL_BIN_VERSION))
				{
					jit = true;
				}
				else
				{
					uint64_t const runtime_file_timestamp = runtime_file->Timestamp();
					uint64_t const input_file_timestamp = res_loader.Timestamp(model_name);
					uint64_t const metadata_timestamp = res_loader.Timestamp(metadata_name);
					if (((input_file_timestamp > 0) && (runtime_file_timestamp < input_file_timestamp))
						|| ((metadata_timestamp > 0) && (runtime_file_timestamp < metadata_timestamp)))
					{
						jit = true;
					}
				}
			}

			if (jit)
			{
#if ZENGINE_IS_DEV_PLATFORM
				RenderFactory& rf = context.RenderFactoryInstance();
				RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();

				return context.DevHelperInstance().ConvertModel(model_name, metadata_name, runtime_name, &caps);
#else
				//LogError() << "Could NOT locate " << runtime_name << std::endl;
				return RenderModelPtr();
#endif
			}
		}
struct NodeInfo
		{
			SceneNodePtr node;
			std::vector<uint16_t> mesh_indices;
			int16_t joint_index;
		};

		std::vector<RenderMaterialPtr> mtls;
		std::vector<VertexElement> merged_ves;
		char all_is_index_16_bit;
		std::vector<std::vector<uint8_t>> merged_buff;
		std::vector<uint8_t> merged_indices;
		std::vector<std::string> mesh_names;
		std::vector<int32_t> mtl_ids;
		std::vector<uint32_t> mesh_lods;
		std::vector<AABBox> pos_bbs;
		std::vector<AABBox> tc_bbs;
		std::vector<uint32_t> mesh_num_vertices;
		std::vector<uint32_t> mesh_base_vertices;
		std::vector<uint32_t> mesh_num_indices;
		std::vector<uint32_t> mesh_start_indices;
		std::vector<NodeInfo> nodes;
		//std::vector<JointComponentPtr> joints;
		//std::shared_ptr<std::vector<Animation>> animations;
		//std::shared_ptr<std::vector<KeyFrameSet>> kfs;
		uint32_t num_frames = 0;
		uint32_t frame_rate = 0;
		//std::vector<std::shared_ptr<AABBKeyFrameSet>> frame_pos_bbs;

		ResIdentifierPtr runtime_file = res_loader.Open(runtime_name);

		uint32_t fourcc;
		runtime_file->read(&fourcc, sizeof(fourcc));
		fourcc = LE2Native(fourcc);
		COMMON_ASSERT((fourcc == MakeFourCC<'K', 'L', 'M', ' '>::value));

		uint32_t ver;
		runtime_file->read(&ver, sizeof(ver));
		ver = LE2Native(ver);
		COMMON_ASSERT(MODEL_BIN_VERSION == ver);

		std::shared_ptr<std::stringstream> ss = MakeSharedPtr<std::stringstream>();

		uint64_t original_len, len;
		runtime_file->read(&original_len, sizeof(original_len));
		original_len = LE2Native(original_len);
		runtime_file->read(&len, sizeof(len));
		len = LE2Native(len);

		LZMACodec lzma;
		lzma.Decode(*ss, runtime_file, len, original_len);

		ResIdentifierPtr decoded = MakeSharedPtr<ResIdentifier>(runtime_file->ResName(), runtime_file->Timestamp(), ss);

		uint32_t num_mtls;
		decoded->read(&num_mtls, sizeof(num_mtls));
		num_mtls = LE2Native(num_mtls);
		uint32_t num_meshes;
		decoded->read(&num_meshes, sizeof(num_meshes));
		num_meshes = LE2Native(num_meshes);
		uint32_t num_nodes;
		decoded->read(&num_nodes, sizeof(num_nodes));
		num_nodes = LE2Native(num_nodes);
		uint32_t num_joints;
		decoded->read(&num_joints, sizeof(num_joints));
		num_joints = LE2Native(num_joints);
		uint32_t num_kfs;
		decoded->read(&num_kfs, sizeof(num_kfs));
		num_kfs = LE2Native(num_kfs);
		uint32_t num_animations;
		decoded->read(&num_animations, sizeof(num_animations));
		num_animations = LE2Native(num_animations);

		mtls.resize(num_mtls);
		for (uint32_t mtl_index = 0; mtl_index < num_mtls; ++ mtl_index)
		{
			RenderMaterialPtr mtl = MakeSharedPtr<RenderMaterial>();
			mtls[mtl_index] = mtl;

			mtl->Name(ReadShortString(*decoded));

			float4 albedo;
			decoded->read(&albedo, sizeof(albedo));
			albedo.x() = LE2Native(albedo.x());
			albedo.y() = LE2Native(albedo.y());
			albedo.z() = LE2Native(albedo.z());
			albedo.w() = LE2Native(albedo.w());
			mtl->Albedo(albedo);

			float metalness;
			decoded->read(&metalness, sizeof(metalness));
			metalness = LE2Native(metalness);
			mtl->Metalness(metalness);

			float glossiness;
			decoded->read(&glossiness, sizeof(glossiness));
			glossiness = LE2Native(glossiness);
			mtl->Glossiness(glossiness);

			float3 emissive;
			decoded->read(&emissive, sizeof(emissive));
			emissive.x() = LE2Native(emissive.x());
			emissive.y() = LE2Native(emissive.y());
			emissive.z() = LE2Native(emissive.z());
			mtl->Emissive(emissive);

			uint8_t transparent;
			decoded->read(&transparent, sizeof(transparent));
			mtl->Transparent(transparent ? true : false);

			uint8_t alpha_test;
			decoded->read(&alpha_test, sizeof(uint8_t));
			mtl->AlphaTestThreshold(alpha_test / 255.0f);

			uint8_t sss;
			decoded->read(&sss, sizeof(sss));
			mtl->Sss(sss ? true : false);

			uint8_t two_sided;
			decoded->read(&two_sided, sizeof(two_sided));
			mtl->TwoSided(two_sided ? true : false);

			for (size_t i = 0; i < RenderMaterial::TS_NumTextureSlots; ++ i)
			{
				mtl->TextureName(static_cast<RenderMaterial::TextureSlot>(i), ReadShortString(*decoded));
			}
			if (!mtl->TextureName(RenderMaterial::TS_Normal).empty())
			{
				float normal_scale;
				decoded->read(&normal_scale, sizeof(normal_scale));
				mtl->NormalScale(LE2Native(normal_scale));
			}
			if (!mtl->TextureName(RenderMaterial::TS_Height).empty())
			{
				float height_offset;
				decoded->read(&height_offset, sizeof(height_offset));
				mtl->HeightOffset(LE2Native(height_offset));
				float height_scale;
				decoded->read(&height_scale, sizeof(height_scale));
				mtl->HeightScale(LE2Native(height_scale));
			}
			if (!mtl->TextureName(RenderMaterial::TS_Occlusion).empty())
			{
				float occlusion_strength;
				decoded->read(&occlusion_strength, sizeof(occlusion_strength));
				mtl->OcclusionStrength(LE2Native(occlusion_strength));
			}

			uint8_t detail_mode;
			decoded->read(&detail_mode, sizeof(detail_mode));
			mtl->DetailMode(static_cast<RenderMaterial::SurfaceDetailMode>(detail_mode));
			if ((mtl->DetailMode() == RenderMaterial::SurfaceDetailMode::FlatTessellation) ||
				(mtl->DetailMode() == RenderMaterial::SurfaceDetailMode::SmoothTessellation))
			{
				float tess_factor;
				decoded->read(&tess_factor, sizeof(tess_factor));
				mtl->EdgeTessHint(LE2Native(tess_factor));
				decoded->read(&tess_factor, sizeof(tess_factor));
				mtl->InsideTessHint(LE2Native(tess_factor));
				decoded->read(&tess_factor, sizeof(tess_factor));
				mtl->MinTessFactor(LE2Native(tess_factor));
				decoded->read(&tess_factor, sizeof(tess_factor));
				mtl->MaxTessFactor(LE2Native(tess_factor));
			}
			else
			{
				mtl->EdgeTessHint(5);
				mtl->InsideTessHint(5);
				mtl->MinTessFactor(1);
				mtl->MaxTessFactor(9);
			}

			mtl->LoadTextureSlots();
		}

		uint32_t num_merged_ves;
		decoded->read(&num_merged_ves, sizeof(num_merged_ves));
		num_merged_ves = LE2Native(num_merged_ves);
		merged_ves.resize(num_merged_ves);
		for (size_t i = 0; i < merged_ves.size(); ++ i)
		{
			decoded->read(&merged_ves[i], sizeof(merged_ves[i]));

			merged_ves[i].usage = LE2Native(merged_ves[i].usage);
			merged_ves[i].format = LE2Native(merged_ves[i].format);
		}

		uint32_t all_num_vertices;
		uint32_t all_num_indices;
		decoded->read(&all_num_vertices, sizeof(all_num_vertices));
		all_num_vertices = LE2Native(all_num_vertices);
		decoded->read(&all_num_indices, sizeof(all_num_indices));
		all_num_indices = LE2Native(all_num_indices);
		decoded->read(&all_is_index_16_bit, sizeof(all_is_index_16_bit));

		int const index_elem_size = all_is_index_16_bit ? 2 : 4;

		merged_buff.resize(merged_ves.size());
		for (size_t i = 0; i < merged_buff.size(); ++ i)
		{
			merged_buff[i].resize(all_num_vertices * merged_ves[i].element_size());
			decoded->read(&merged_buff[i][0], merged_buff[i].size() * sizeof(merged_buff[i][0]));
		}
		merged_indices.resize(all_num_indices * index_elem_size);
		decoded->read(&merged_indices[0], merged_indices.size() * sizeof(merged_indices[0]));

		mesh_names.resize(num_meshes);
		mtl_ids.resize(num_meshes);
		mesh_lods.resize(num_meshes);
		pos_bbs.resize(num_meshes);
		tc_bbs.resize(num_meshes);
		mesh_num_vertices.clear();
		mesh_base_vertices.clear();
		mesh_num_indices.clear();
		mesh_start_indices.clear();
		for (uint32_t mesh_index = 0; mesh_index < num_meshes; ++ mesh_index)
		{
			mesh_names[mesh_index] = ReadShortString(*decoded);

			decoded->read(&mtl_ids[mesh_index], sizeof(mtl_ids[mesh_index]));
			mtl_ids[mesh_index] = LE2Native(mtl_ids[mesh_index]);

			decoded->read(&mesh_lods[mesh_index], sizeof(mesh_lods[mesh_index]));
			mesh_lods[mesh_index] = LE2Native(mesh_lods[mesh_index]);

			float3 min_bb, max_bb;
			decoded->read(&min_bb, sizeof(min_bb));
			min_bb.x() = LE2Native(min_bb.x());
			min_bb.y() = LE2Native(min_bb.y());
			min_bb.z() = LE2Native(min_bb.z());
			decoded->read(&max_bb, sizeof(max_bb));
			max_bb.x() = LE2Native(max_bb.x());
			max_bb.y() = LE2Native(max_bb.y());
			max_bb.z() = LE2Native(max_bb.z());
			pos_bbs[mesh_index] = AABBox(min_bb, max_bb);

			decoded->read(&min_bb[0], sizeof(min_bb[0]));
			decoded->read(&min_bb[1], sizeof(min_bb[1]));
			min_bb.x() = LE2Native(min_bb.x());
			min_bb.y() = LE2Native(min_bb.y());
			min_bb.z() = 0;
			decoded->read(&max_bb[0], sizeof(max_bb[0]));
			decoded->read(&max_bb[1], sizeof(max_bb[1]));
			max_bb.x() = LE2Native(max_bb.x());
			max_bb.y() = LE2Native(max_bb.y());
			max_bb.z() = 0;
			tc_bbs[mesh_index] = AABBox(min_bb, max_bb);

			for (uint32_t lod = 0; lod < mesh_lods[mesh_index]; ++ lod)
			{
				uint32_t tmp;
				decoded->read(&tmp, sizeof(tmp));
				mesh_num_vertices.push_back(LE2Native(tmp));
				decoded->read(&tmp, sizeof(tmp));
				mesh_base_vertices.push_back(LE2Native(tmp));
				decoded->read(&tmp, sizeof(tmp));
				mesh_num_indices.push_back(LE2Native(tmp));
				decoded->read(&tmp, sizeof(tmp));
				mesh_start_indices.push_back(LE2Native(tmp));
			}
		}

		nodes.resize(num_nodes);
		for (auto& node : nodes)
		{
			auto node_name = ReadShortString(*decoded);
			std::wstring node_wname;
			Convert(node_wname, node_name);

			uint32_t attrib;
			decoded->read(&attrib, sizeof(attrib));

			node.node = MakeSharedPtr<SceneNode>(node_wname, attrib);

			int16_t parent_index;
			decoded->read(&parent_index, sizeof(parent_index));
			parent_index = LE2Native(parent_index);

			if (parent_index >= 0)
			{
				nodes[parent_index].node->AddChild(node.node);
			}

			uint16_t num_mesh_indices;
			decoded->read(&num_mesh_indices, sizeof(num_mesh_indices));
			num_mesh_indices = LE2Native(num_mesh_indices);

			if (num_mesh_indices > 0)
			{
				node.mesh_indices.resize(num_mesh_indices);
				decoded->read(node.mesh_indices.data(), node.mesh_indices.size() * sizeof(node.mesh_indices[0]));
				for (auto& mesh_index : node.mesh_indices)
				{
					mesh_index = LE2Native(mesh_index);
				}
			}

			int16_t joint_index;
			decoded->read(&joint_index, sizeof(joint_index));
			node.joint_index = LE2Native(joint_index);

			float4x4 xform_to_parent;
			decoded->read(&xform_to_parent, sizeof(xform_to_parent));
			for (auto& item : xform_to_parent)
			{
				item = LE2Native(item);
			}
			node.node->TransformToParent(xform_to_parent);
		}

		// joints.resize(num_joints);
		// for (uint32_t joint_index = 0; joint_index < num_joints; ++ joint_index)
		// {
		// 	joints[joint_index] = MakeSharedPtr<JointComponent>();
		// 	JointComponent& joint = *joints[joint_index];

		// 	Quaternion bind_real;
		// 	decoded->read(&bind_real, sizeof(bind_real));
		// 	bind_real[0] = LE2Native(bind_real[0]);
		// 	bind_real[1] = LE2Native(bind_real[1]);
		// 	bind_real[2] = LE2Native(bind_real[2]);
		// 	bind_real[3] = LE2Native(bind_real[3]);

		// 	Quaternion bind_dual;
		// 	decoded->read(&bind_dual, sizeof(bind_dual));
		// 	bind_dual[0] = LE2Native(bind_dual[0]);
		// 	bind_dual[1] = LE2Native(bind_dual[1]);
		// 	bind_dual[2] = LE2Native(bind_dual[2]);
		// 	bind_dual[3] = LE2Native(bind_dual[3]);

		// 	float flip = MathLib::SignBit(bind_real.w());

		// 	float bind_scale = MathLib::length(bind_real);
		// 	float inverse_origin_scale = 1 / bind_scale;
		// 	bind_real *= inverse_origin_scale;

		// 	Quaternion inverse_origin_real, inverse_origin_dual;
		// 	if (flip > 0)
		// 	{
		// 		std::tie(inverse_origin_real, inverse_origin_dual) = MathLib::inverse(bind_real, bind_dual);
		// 	}
		// 	else
		// 	{
		// 		float4x4 tmp_mat = MathLib::scaling(bind_scale, bind_scale, flip * bind_scale)
		// 			* MathLib::to_matrix(bind_real)
		// 			* MathLib::translation(MathLib::udq_to_trans(bind_real, bind_dual));
		// 		tmp_mat = MathLib::inverse(tmp_mat);
		// 		tmp_mat(2, 0) = -tmp_mat(2, 0);
		// 		tmp_mat(2, 1) = -tmp_mat(2, 1);
		// 		tmp_mat(2, 2) = -tmp_mat(2, 2);

		// 		float3 scale;
		// 		Quaternion rot;
		// 		float3 trans;
		// 		MathLib::decompose(scale, rot, trans, tmp_mat);

		// 		inverse_origin_real = rot;
		// 		inverse_origin_dual = MathLib::quat_trans_to_udq(rot, trans);
		// 		inverse_origin_scale = -scale.x();
		// 	}

		// 	bind_scale *= flip;

		// 	joint.BindParams(bind_real, bind_dual, bind_scale);
		// 	joint.InverseOriginParams(inverse_origin_real, inverse_origin_dual, inverse_origin_scale);
		// }

		if (num_kfs > 0)
		{
			decoded->read(&num_frames, sizeof(num_frames));
			num_frames = LE2Native(num_frames);
			decoded->read(&frame_rate, sizeof(frame_rate));
			frame_rate = LE2Native(frame_rate);

			// kfs = MakeSharedPtr<std::vector<KeyFrameSet>>(joints.size());
			// for (uint32_t kf_index = 0; kf_index < num_kfs; ++ kf_index)
			// {
			// 	uint32_t joint_index = kf_index;

			// 	uint32_t num_kf;
			// 	decoded->read(&num_kf, sizeof(num_kf));
			// 	num_kf = LE2Native(num_kf);

			// 	KeyFrameSet kf;
			// 	kf.frame_id.resize(num_kf);
			// 	kf.bind_real.resize(num_kf);
			// 	kf.bind_dual.resize(num_kf);
			// 	kf.bind_scale.resize(num_kf);
			// 	for (uint32_t k_index = 0; k_index < num_kf; ++ k_index)
			// 	{
			// 		decoded->read(&kf.frame_id[k_index], sizeof(kf.frame_id[k_index]));
			// 		kf.frame_id[k_index] = LE2Native(kf.frame_id[k_index]);
			// 		decoded->read(&kf.bind_real[k_index], sizeof(kf.bind_real[k_index]));
			// 		kf.bind_real[k_index][0] = LE2Native(kf.bind_real[k_index][0]);
			// 		kf.bind_real[k_index][1] = LE2Native(kf.bind_real[k_index][1]);
			// 		kf.bind_real[k_index][2] = LE2Native(kf.bind_real[k_index][2]);
			// 		kf.bind_real[k_index][3] = LE2Native(kf.bind_real[k_index][3]);
			// 		decoded->read(&kf.bind_dual[k_index], sizeof(kf.bind_dual[k_index]));
			// 		kf.bind_dual[k_index][0] = LE2Native(kf.bind_dual[k_index][0]);
			// 		kf.bind_dual[k_index][1] = LE2Native(kf.bind_dual[k_index][1]);
			// 		kf.bind_dual[k_index][2] = LE2Native(kf.bind_dual[k_index][2]);
			// 		kf.bind_dual[k_index][3] = LE2Native(kf.bind_dual[k_index][3]);

			// 		float flip = MathLib::SignBit(kf.bind_real[k_index].w());

			// 		kf.bind_scale[k_index] = MathLib::length(kf.bind_real[k_index]);
			// 		kf.bind_real[k_index] /= kf.bind_scale[k_index];

			// 		kf.bind_scale[k_index] *= flip;
			// 	}

			// 	if (joint_index < num_joints)
			// 	{
			// 		(*kfs)[joint_index] = kf;
			// 	}
			// }

			//frame_pos_bbs.resize(num_meshes);
			// for (uint32_t mesh_index = 0; mesh_index < num_meshes; ++ mesh_index)
			// {
			// 	uint32_t num_bb_kf;
			// 	decoded->read(&num_bb_kf, sizeof(num_bb_kf));
			// 	num_bb_kf = LE2Native(num_bb_kf);

			// 	frame_pos_bbs[mesh_index] = MakeSharedPtr<AABBKeyFrameSet>();
			// 	frame_pos_bbs[mesh_index]->frame_id.resize(num_bb_kf);
			// 	frame_pos_bbs[mesh_index]->bb.resize(num_bb_kf);

			// 	for (uint32_t bb_k_index = 0; bb_k_index < num_bb_kf; ++ bb_k_index)
			// 	{
			// 		decoded->read(&frame_pos_bbs[mesh_index]->frame_id[bb_k_index], sizeof(frame_pos_bbs[mesh_index]->frame_id[bb_k_index]));
			// 		frame_pos_bbs[mesh_index]->frame_id[bb_k_index] = LE2Native(frame_pos_bbs[mesh_index]->frame_id[bb_k_index]);

			// 		float3 bb_min, bb_max;
			// 		decoded->read(&bb_min, sizeof(bb_min));
			// 		bb_min[0] = LE2Native(bb_min[0]);
			// 		bb_min[1] = LE2Native(bb_min[1]);
			// 		bb_min[2] = LE2Native(bb_min[2]);
			// 		decoded->read(&bb_max, sizeof(bb_max));
			// 		bb_max[0] = LE2Native(bb_max[0]);
			// 		bb_max[1] = LE2Native(bb_max[1]);
			// 		bb_max[2] = LE2Native(bb_max[2]);
			// 		frame_pos_bbs[mesh_index]->bb[bb_k_index] = AABBox(bb_min, bb_max);
			// 	}
			// }

			// if (num_animations > 0)
			// {
			// 	animations = MakeSharedPtr<std::vector<Animation>>(num_animations);
			// 	for (uint32_t animation_index = 0; animation_index < num_animations; ++animation_index)
			// 	{
			// 		Animation animation;
			// 		animation.name = ReadShortString(*decoded);
			// 		decoded->read(&animation.start_frame, sizeof(animation.start_frame));
			// 		animation.start_frame = LE2Native(animation.start_frame);
			// 		decoded->read(&animation.end_frame, sizeof(animation.end_frame));
			// 		animation.end_frame = LE2Native(animation.end_frame);
			// 		(*animations)[animation_index] = animation;
			// 	}
			// }
		}

		bool const skinned = false;//kfs && !kfs->empty();

		RenderModelPtr model;
		if (skinned)
		{
			model = MakeSharedPtr<SkinnedModel>(nodes[0].node);
		}
		else
		{
			model = MakeSharedPtr<RenderModel>(nodes[0].node);
		}

		model->NumMaterials(mtls.size());
		for (uint32_t mtl_index = 0; mtl_index < mtls.size(); ++ mtl_index)
		{
			model->GetMaterial(mtl_index) = mtls[mtl_index];
		}

		std::vector<GraphicsBufferPtr> merged_vbs(merged_buff.size());
		for (size_t i = 0; i < merged_buff.size(); ++ i)
		{
			auto vb = MakeSharedPtr<SoftwareGraphicsBuffer>(static_cast<uint32_t>(merged_buff[i].size()), false);
			vb->CreateHWResource(merged_buff[i].data());

			merged_vbs[i] = vb;
		}
		auto merged_ib = MakeSharedPtr<SoftwareGraphicsBuffer>(static_cast<uint32_t>(merged_indices.size()), false);
		merged_ib->CreateHWResource(merged_indices.data());

		uint32_t mesh_lod_index = 0;
		std::vector<StaticMeshPtr> meshes(num_meshes);
		for (uint32_t mesh_index = 0; mesh_index < num_meshes; ++ mesh_index)
		{
			std::wstring wname;
			Convert(wname, mesh_names[mesh_index]);

			if (skinned)
			{
				//meshes[mesh_index] = MakeSharedPtr<SkinnedMesh>(wname);
			}
			else
			{
				meshes[mesh_index] = MakeSharedPtr<StaticMesh>(wname);
			}
			StaticMeshPtr& mesh = meshes[mesh_index];

			mesh->MaterialID(mtl_ids[mesh_index]);
			mesh->PosBound(pos_bbs[mesh_index]);
			mesh->TexcoordBound(tc_bbs[mesh_index]);

			uint32_t const lods = mesh_lods[mesh_index];
			mesh->NumLods(lods);
			for (uint32_t lod = 0; lod < lods; ++ lod, ++ mesh_lod_index)
			{
				for (uint32_t ve_index = 0; ve_index < merged_buff.size(); ++ ve_index)
				{
					mesh->AddVertexStream(lod, merged_vbs[ve_index], merged_ves[ve_index]);
				}
				mesh->AddIndexStream(lod, merged_ib, all_is_index_16_bit ? EF_R16UI : EF_R32UI);

				mesh->NumVertices(lod, mesh_num_vertices[mesh_lod_index]);
				mesh->NumIndices(lod, mesh_num_indices[mesh_lod_index]);
				mesh->StartVertexLocation(lod, mesh_base_vertices[mesh_lod_index]);
				mesh->StartIndexLocation(lod, mesh_start_indices[mesh_lod_index]);
			}
		}

		// if (kfs && !kfs->empty())
		// {
		// 	if (!joints.empty())
		// 	{
		// 		SkinnedModelPtr skinned_model = checked_pointer_cast<SkinnedModel>(model);

		// 		skinned_model->AssignJoints(joints.begin(), joints.end());
		// 		skinned_model->AttachKeyFrameSets(kfs);

		// 		skinned_model->NumFrames(num_frames);
		// 		skinned_model->FrameRate(frame_rate);

		// 		for (size_t mesh_index = 0; mesh_index < meshes.size(); ++ mesh_index)
		// 		{
		// 			SkinnedMeshPtr skinned_mesh = checked_pointer_cast<SkinnedMesh>(meshes[mesh_index]);
		// 			skinned_mesh->AttachFramePosBounds(frame_pos_bbs[mesh_index]);
		// 		}

		// 		skinned_model->AttachAnimations(animations);
		// 	}
		// }

		// model->AssignMeshes(meshes.begin(), meshes.end());
		// for (auto const& node : nodes)
		// {
		// 	for (auto const mesh_index : node.mesh_indices)
		// 	{
		// 		node.node->AddComponent(MakeSharedPtr<RenderableComponent>(meshes[mesh_index]));
		// 	}

		// 	if (node.joint_index >= 0)
		// 	{
		// 		node.node->AddComponent(joints[node.joint_index]);
		// 	}
		// }

		model->RootNode()->UpdatePosBoundSubtree();

		return model;
	}

    void SaveModel(RenderModel const & model, std::string_view model_name)
    {
		std::filesystem::path output_path(model_name);
		auto const output_ext = output_path.extension().string();
		bool need_conversion = false;
		if (output_ext != ".model_bin")
		{
			output_path += ".model_bin";
			need_conversion = true;
		}

		std::vector<RenderMaterialPtr> mtls(model.NumMaterials());
		if (!mtls.empty())
		{
			for (uint32_t i = 0; i < mtls.size(); ++ i)
			{
				mtls[i] = model.GetMaterial(i);
			}
		}

		std::vector<VertexElement> merged_ves;
		std::vector<std::vector<uint8_t>> merged_buffs;
		char all_is_index_16_bit = false;
		std::vector<uint8_t> merged_indices;
		std::vector<std::string> mesh_names(model.NumMeshes());
		std::vector<int32_t> mtl_ids(mesh_names.size());
		std::vector<uint32_t> mesh_lods(mesh_names.size());
		std::vector<AABBox> pos_bbs(mesh_names.size());
		std::vector<AABBox> tc_bbs(mesh_names.size());
		std::vector<uint32_t> mesh_num_vertices;
		std::vector<uint32_t> mesh_base_vertices;
		std::vector<uint32_t> mesh_num_indices;
		std::vector<uint32_t> mesh_base_indices;
		if (!mesh_names.empty())
		{
			{
				auto const& mesh = checked_cast<StaticMesh&>(*model.Mesh(0));

				RenderLayout const & rl = mesh.GetRenderLayout();
				merged_ves.resize(rl.VertexStreamNum());
				for (uint32_t j = 0; j < rl.VertexStreamNum(); ++ j)
				{
					merged_ves[j] = rl.VertexStreamFormat(j)[0];
				}

				merged_buffs.resize(merged_ves.size());
				for (uint32_t j = 0; j < rl.VertexStreamNum(); ++ j)
				{
					GraphicsBufferPtr const & vb = rl.GetVertexStream(j);
					uint32_t size = vb->Size();
					GraphicsBufferPtr vb_cpu;
					if (vb->AccessHint() & EAH_CPU_Read)
					{
						vb_cpu = vb;
					}
					else
					{
						auto& rf = Context::Instance().RenderFactoryInstance();
						vb_cpu = rf.MakeVertexBuffer(BU_Static, EAH_CPU_Read, size, nullptr);
						vb->CopyToBuffer(*vb_cpu);
					}

					merged_buffs[j].resize(size);

					GraphicsBuffer::Mapper mapper(*vb_cpu, BA_Read_Only);
					std::memcpy(&merged_buffs[j][0], mapper.Pointer<uint8_t>(), size);
				}

				if (EF_R16UI == rl.IndexStreamFormat())
				{
					all_is_index_16_bit = true;
				}
				else
				{
					COMMON_ASSERT(EF_R32UI == rl.IndexStreamFormat());
					all_is_index_16_bit = false;
				}

				{
					GraphicsBufferPtr const & ib = rl.GetIndexStream();
					uint32_t size = ib->Size();
					GraphicsBufferPtr ib_cpu;
					if (ib->AccessHint() & EAH_CPU_Read)
					{
						ib_cpu = ib;
					}
					else
					{
						auto& rf = Context::Instance().RenderFactoryInstance();
						ib_cpu = rf.MakeIndexBuffer(BU_Static, EAH_CPU_Read, size, nullptr);
						ib->CopyToBuffer(*ib_cpu);
					}

					merged_indices.resize(size);

					GraphicsBuffer::Mapper mapper(*ib_cpu, BA_Read_Only);
					std::memcpy(&merged_indices[0], mapper.Pointer<uint8_t>(), size);
				}
			}

			for (uint32_t mesh_index = 0; mesh_index < mesh_names.size(); ++ mesh_index)
			{
				auto const& mesh = checked_cast<StaticMesh&>(*model.Mesh(mesh_index));

				Convert(mesh_names[mesh_index], mesh.Name());
				mtl_ids[mesh_index] = mesh.MaterialID();

				mesh_lods[mesh_index] = mesh.NumLods();

				pos_bbs[mesh_index] = mesh.PosBound();
				tc_bbs[mesh_index] = mesh.TexcoordBound();

				for (uint32_t lod = 0; lod < mesh_lods[mesh_index]; ++ lod)
				{
					mesh_num_vertices.push_back(mesh.NumVertices(lod));
					mesh_base_vertices.push_back(mesh.StartVertexLocation(lod));
					mesh_num_indices.push_back(mesh.NumIndices(lod));
					mesh_base_indices.push_back(mesh.StartIndexLocation(lod));
				}
			}

			mesh_base_vertices.push_back(mesh_base_vertices.back() + mesh_num_vertices.back());
			mesh_base_indices.push_back(mesh_base_indices.back() + mesh_num_indices.back());
		}

		std::vector<SceneNode const *> nodes;
		model.RootNode()->Traverse([&nodes](SceneNode& node)
			{
				nodes.push_back(&node);
				return true;
			});

		std::vector<Renderable const *> renderables;
		for (uint32_t mesh_index = 0; mesh_index < mesh_names.size(); ++ mesh_index)
		{
			renderables.push_back(model.Mesh(mesh_index).get());
		}

		// std::vector<JointComponent const*> joints;
		// std::shared_ptr<std::vector<Animation>> animations;
		// std::shared_ptr<std::vector<KeyFrameSet>> kfs;
		// uint32_t num_frame = 0;
		// uint32_t frame_rate = 0;
		// std::vector<std::shared_ptr<AABBKeyFrameSet>> frame_pos_bbs;
		// if (model.IsSkinned())
		// {
		// 	auto const& skinned_model = checked_cast<SkinnedModel const&>(model);

		// 	uint32_t num_joints = skinned_model.NumJoints();
		// 	joints.resize(num_joints);
		// 	for (uint32_t i = 0; i < num_joints; ++ i)
		// 	{
		// 		joints[i] = skinned_model.GetJoint(i).get();
		// 	}

		// 	animations = skinned_model.GetAnimations();

		// 	num_frame = skinned_model.NumFrames();
		// 	frame_rate = skinned_model.FrameRate();

		// 	kfs = skinned_model.GetKeyFrameSets();

		// 	frame_pos_bbs.resize(mesh_names.size());
		// 	for (uint32_t mesh_index = 0; mesh_index < mesh_names.size(); ++ mesh_index)
		// 	{
		// 		auto& skinned_mesh = checked_cast<SkinnedMesh&>(*skinned_model.Mesh(mesh_index));
		// 		frame_pos_bbs[mesh_index] = skinned_mesh.GetFramePosBounds();
		// 	}
		// }

#if ZENGINE_IS_DEV_PLATFORM
		if (need_conversion)
		{
			RenderDeviceCaps const * caps = nullptr;
			if (Context::Instance().RenderFactoryValid())
			{
				RenderFactory& rf = Context::Instance().RenderFactoryInstance();
				caps = &rf.RenderEngineInstance().DeviceCaps();
			}

			//Context::Instance().DevHelperInstance().ConvertModel(output_path.string(), "", model_name, caps);
		}
#endif
    }
}
