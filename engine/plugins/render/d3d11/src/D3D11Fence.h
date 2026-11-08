#pragma once
#include <render/Fence.h>
#include <base/SmartPtrHelper.h>

#include "D3D11Util.h"

namespace RenderWorker
{
	class D3D11Fence final : public Fence
	{
	public:
		D3D11Fence();

		virtual uint64_t Signal(FenceType ft) override;
		virtual void Wait(uint64_t id) override;
		virtual bool Completed(uint64_t id) override;

	private:
		std::map<uint64_t, ID3D11QueryPtr> fences_;
		std::atomic<uint64_t> fence_val_{0};
	};
	
	class D3D11_4Fence final : public Fence
	{
	public:
		D3D11_4Fence();

		uint64_t Signal(FenceType ft) override;
		void Wait(uint64_t id) override;
		bool Completed(uint64_t id) override;

	private:
		ID3D11FencePtr fence_;
		Win32UniqueHandle fence_event_;
		uint64_t last_completed_val_{0};
		std::atomic<uint64_t> fence_val_{1};
	};

}