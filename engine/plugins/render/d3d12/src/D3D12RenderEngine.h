#pragma once

#include <render/RenderEngine.h>
#include "D3D12Util.h"
#include <math/color.h>
#include <array>
#include <memory>
#include <unordered_map>
#include <vector>

namespace RenderWorker
{

class D3D12ShaderStageObject;

struct D3D12DeviceLifetime final
{
	d3d12::Device* device{nullptr};
};

class ZENGINE_D3D12_API D3D12RenderEngine : public RenderEngine
{
public:
	D3D12RenderEngine();
	~D3D12RenderEngine() override;

	bool RequiresFlipping() const override
	{
		return false;
	}

	void BeginFrame() override;
	void EndFrame() override;
	void BeginOverlayPass() override;
	bool CaptureCurrentColor(TexturePtr& snapshot) override;
	void ScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

	d3d12::Device* Device() const noexcept
	{
		return device_;
	}
	std::shared_ptr<D3D12DeviceLifetime> const& DeviceLifetime() const noexcept
	{
		return device_lifetime_;
	}
	void* Window() const noexcept
	{
		return window_;
	}
	d3d12::CommandBuffer* CurrentCommandBuffer() const noexcept
	{
		return cmd_;
	}

	void Device(d3d12::Device* device, void* window);
	void DetachWindow() noexcept
	{
		window_ = nullptr;
	}
	void FillRenderDeviceCaps();

	void CurrentCommandBuffer(d3d12::CommandBuffer* cmd) noexcept
	{
		cmd_ = cmd;
	}
	void CurrentSwapchainTexture(d3d12::Texture* tex) noexcept
	{
		swapchain_tex_ = tex;
	}
	d3d12::Texture* CurrentSwapchainTexture() const noexcept
	{
		return swapchain_tex_;
	}

	d3d12::RenderPass* CurrentRenderPass() const noexcept
	{
		return render_pass_;
	}

	void EnsureRenderPass(bool clear_color, Color const* clear_clr, bool clear_depth, float depth, bool clear_stencil,
		int32_t stencil);
	void EndRenderPass();

private:
	void DoCreateRenderWindow(std::string const& name, RenderSettings const& settings) override;
	void DoRender(const RenderEffect& effect, const RenderTechnique& tech, const RenderLayout& rl) override;
	void DoBindFrameBuffer(FrameBufferPtr const& fb) override;
	void DoBindSOBuffers(const RenderLayoutPtr& rl) override;
	void DoDestroy() override;

	d3d12::GraphicsPipeline* GetOrCreatePipeline(const RenderEffect& effect, const RenderPass& pass,
		const RenderLayout& rl);
	void EnsureDefaultSampleBinding();
    d3d12::Texture* DefaultTexture(d3d12::TextureType type);
	void BindStageSamplers(RenderEffect const& effect, D3D12ShaderStageObject const& stage, bool fragment);
	void ResolvePassTargets();
	void ApplyViewportAndScissor();

private:
	std::shared_ptr<D3D12DeviceLifetime> device_lifetime_{std::make_shared<D3D12DeviceLifetime>()};
	d3d12::Device* device_{nullptr};
	void* window_{nullptr};
	d3d12::CommandBuffer* cmd_{nullptr};
	d3d12::Texture* swapchain_tex_{nullptr};
	d3d12::RenderPass* render_pass_{nullptr};
	d3d12::Sampler* default_sampler_{nullptr};
	std::array<d3d12::Texture*, d3d12::TEXTURETYPE_CUBE_ARRAY + 1> default_textures_{};

	uint32_t scissor_x_{0}, scissor_y_{0}, scissor_w_{0}, scissor_h_{0};
	bool scissor_valid_{false};
	bool scissor_enabled_{false};

	struct ColorTarget
	{
		d3d12::Texture* texture{nullptr};
		d3d12::TextureFormat format{d3d12::TEXTUREFORMAT_INVALID};
		uint32_t mip_level{0};
		uint32_t layer{0};
	};
	std::vector<ColorTarget> pass_colors_;
	d3d12::Texture* pass_depth_{nullptr};
	d3d12::TextureFormat pass_depth_format_{d3d12::TEXTUREFORMAT_INVALID};
	bool pass_has_depth_{false};

	static constexpr size_t kMaxPipelineColorTargets = 8;

	struct PipelineKey
	{
		d3d12::Shader* vs{nullptr};
		d3d12::Shader* ps{nullptr};
		void const* layout{nullptr};
		void const* state{nullptr};
		RenderLayout::topology_type topo{RenderLayout::TT_TriangleList};
		uint32_t num_colors{0};
		std::array<d3d12::TextureFormat, kMaxPipelineColorTargets> color_fmts{};
		bool has_depth{false};
		d3d12::TextureFormat depth_fmt{d3d12::TEXTUREFORMAT_INVALID};

		bool operator==(PipelineKey const& o) const noexcept
		{
			if (!(vs == o.vs && ps == o.ps && layout == o.layout && state == o.state && topo == o.topo
					&& num_colors == o.num_colors && has_depth == o.has_depth && depth_fmt == o.depth_fmt))
			{
				return false;
			}
			for (uint32_t i = 0; i < num_colors; ++i)
			{
				if (color_fmts[i] != o.color_fmts[i])
				{
					return false;
				}
			}
			return true;
		}
	};
	struct PipelineKeyHash
	{
		size_t operator()(PipelineKey const& k) const noexcept
		{
			size_t h = reinterpret_cast<size_t>(k.vs);
			h ^= reinterpret_cast<size_t>(k.ps) + 0x9e3779b9 + (h << 6) + (h >> 2);
			h ^= reinterpret_cast<size_t>(k.layout) + 0x9e3779b9 + (h << 6) + (h >> 2);
			h ^= reinterpret_cast<size_t>(k.state) + 0x9e3779b9 + (h << 6) + (h >> 2);
			h ^= static_cast<size_t>(k.topo) + 0x9e3779b9 + (h << 6) + (h >> 2);
			h ^= static_cast<size_t>(k.num_colors) + 0x9e3779b9 + (h << 6) + (h >> 2);
			h ^= static_cast<size_t>(k.has_depth) + 0x9e3779b9 + (h << 6) + (h >> 2);
			h ^= static_cast<size_t>(k.depth_fmt) + 0x9e3779b9 + (h << 6) + (h >> 2);
			for (uint32_t i = 0; i < k.num_colors; ++i)
			{
				h ^= static_cast<size_t>(k.color_fmts[i]) + 0x9e3779b9 + (h << 6) + (h >> 2);
			}
			return h;
		}
	};
	std::unordered_map<PipelineKey, d3d12::GraphicsPipeline*, PipelineKeyHash> pipelines_;
};

} // namespace RenderWorker
