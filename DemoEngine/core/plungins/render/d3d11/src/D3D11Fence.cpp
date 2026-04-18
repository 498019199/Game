#include "D3D11Fence.h"
#include "D3D11RenderFactory.h"
#include "D3D11RenderEngine.h"
#include <base/ZEngine.h>

namespace RenderWorker
{
	D3D11Fence::D3D11Fence() = default;

	uint64_t D3D11Fence::Signal([[maybe_unused]] FenceType ft)
	{
		auto const& re = checked_cast<D3D11RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11Device1* d3d_device = re.D3DDevice1();
		ID3D11DeviceContext1* d3d_imm_ctx = re.D3DDeviceImmContext1();

		D3D11_QUERY_DESC desc;
		desc.Query = D3D11_QUERY_EVENT;
		desc.MiscFlags = 0;

		ID3D11QueryPtr query;
		d3d_device->CreateQuery(&desc, query.put());

		uint64_t const id = fence_val_;
		++ fence_val_;

		d3d_imm_ctx->End(query.get());

		fences_[id] = std::move(query);

		return id;
	}

	void D3D11Fence::Wait(uint64_t id)
	{
		auto iter = fences_.find(id);
		if (iter != fences_.end())
		{
			auto const& re = checked_cast<D3D11RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			ID3D11DeviceContext1* d3d_imm_ctx = re.D3DDeviceImmContext1();

			uint32_t ret;
			while (S_OK != d3d_imm_ctx->GetData(iter->second.get(), &ret, sizeof(ret), 0));
			fences_.erase(iter);
		}
	}

	bool D3D11Fence::Completed(uint64_t id)
	{
		auto iter = fences_.find(id);
		if (iter == fences_.end())
		{
			return true;
		}
		else
		{
			auto const& re = checked_cast<D3D11RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			ID3D11DeviceContext1* d3d_imm_ctx = re.D3DDeviceImmContext1();

			uint32_t ret;
			HRESULT hr = d3d_imm_ctx->GetData(iter->second.get(), &ret, sizeof(ret), 0);
			return (S_OK == hr);
		}
	}


	D3D11_4Fence::D3D11_4Fence()
	{
		auto const& re = checked_cast<D3D11RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		auto* d3d_device = re.D3DDevice5();
		COMMON_ASSERT(d3d_device != nullptr);

		d3d_device->CreateFence(0, D3D11_FENCE_FLAG_NONE, UuidOf<ID3D11Fence>(), fence_.put_void());

		fence_event_ = MakeWin32UniqueHandle(::CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS));
	}

	uint64_t D3D11_4Fence::Signal([[maybe_unused]] FenceType ft)
	{
		auto const& re = checked_cast<D3D11RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		auto* d3d_imm_ctx = re.D3DDeviceImmContext4();
		COMMON_ASSERT(d3d_imm_ctx != nullptr);

		uint64_t const id = fence_val_;
		d3d_imm_ctx->Signal(fence_.get(), id);
		++ fence_val_;

		return id;
	}

	void D3D11_4Fence::Wait(uint64_t id)
	{
		if (!this->Completed(id))
		{
			TIFHR(fence_->SetEventOnCompletion(id, fence_event_.get()));
			::WaitForSingleObjectEx(fence_event_.get(), INFINITE, FALSE);
		}
	}

	bool D3D11_4Fence::Completed(uint64_t id)
	{
		if (id > last_completed_val_)
		{
			last_completed_val_ = std::max(last_completed_val_, fence_->GetCompletedValue());
		}
		return id <= last_completed_val_;
	}
}
