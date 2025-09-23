#include <render/Mesh.h>
#include <common/Util.h>
#include <base/ResLoader.h>
#include <render/RenderFactory.h>

namespace
{
	using namespace RenderWorker;
	using namespace CommonWorker;

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


namespace RenderWorker
{
	StaticMesh::StaticMesh(std::wstring_view name)
		: Renderable(name), hw_res_ready_(false)
	{
	}

	void StaticMesh::BuildMeshInfo(const RenderModel& model)
	{
		this->DoBuildMeshInfo(model);
		//this->UpdateBoundBox();

		hw_res_ready_ = true;
	}

	void StaticMesh::DoBuildMeshInfo(RenderModel const & model)
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

		//this->IsSkinned(is_skinned);
		this->Material(model.GetMaterial(this->MaterialID()));
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



	void RenderModel::BuildModelInfo()
	{
		//this->DoBuildModelInfo();
		//root_node_->UpdatePosBoundSubtree();

		hw_res_ready_ = true;
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
}

namespace RenderWorker
{
	using namespace CommonWorker;

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

    void SaveModel(RenderModel const & model, std::string_view model_name)
    {

    }
}
