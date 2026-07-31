#pragma once

#include <render/RenderEngine.h>
#include "SDL3Util.h"
#include "SDL3MvpTriangle.h"
#include "SDL3SkyBoxPresent.h"
#include "SDL3MeshPresent.h"
#include <math/color.h>
#include <array>
#include <memory>
#include <unordered_map>
#include <vector>

namespace RenderWorker
{

class SDL3ShaderStageObject;

class SDL3RenderEngine : public RenderEngine
{
public:
	SDL3RenderEngine();
	~SDL3RenderEngine() override;

	bool RequiresFlipping() const override
	{
		// D3D11/D3D12 SDL_GPU backends match D3D NDC (no Y flip). Vulkan/Metal may differ later.
		return false;
	}

	void BeginFrame() override;
	void EndFrame() override;
	void ScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

	SDL_GPUDevice* Device() const noexcept
	{
		return device_;
	}
	SDL_Window* Window() const noexcept
	{
		return window_;
	}
	SDL_GPUCommandBuffer* CurrentCommandBuffer() const noexcept
	{
		return cmd_;
	}

	void Device(SDL_GPUDevice* device, SDL_Window* window);
	void DetachWindow() noexcept
	{
		window_ = nullptr;
	}
	void FillRenderDeviceCaps();

	void CurrentCommandBuffer(SDL_GPUCommandBuffer* cmd) noexcept
	{
		cmd_ = cmd;
	}
	void CurrentSwapchainTexture(SDL_GPUTexture* tex) noexcept
	{
		swapchain_tex_ = tex;
	}
	SDL_GPUTexture* CurrentSwapchainTexture() const noexcept
	{
		return swapchain_tex_;
	}

	SDL_GPURenderPass* CurrentRenderPass() const noexcept
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

	SDL_GPUGraphicsPipeline* GetOrCreatePipeline(const RenderEffect& effect, const RenderPass& pass,
		const RenderLayout& rl);
	void EnsureDefaultSampleBinding();
	void BindStageSamplers(RenderEffect const& effect, SDL3ShaderStageObject const& stage, bool fragment);
	void ResolvePassTargets();
	void ApplyViewportAndScissor();

private:
	SDL_GPUDevice* device_{nullptr};
	SDL_Window* window_{nullptr};
	SDL_GPUCommandBuffer* cmd_{nullptr};
	SDL_GPUTexture* swapchain_tex_{nullptr};
	SDL_GPURenderPass* render_pass_{nullptr};
	SDL_GPUSampler* default_sampler_{nullptr};
	SDL_GPUTexture* default_sample_tex_{nullptr};
	bool owns_sdl_init_{false};

	uint32_t scissor_x_{0}, scissor_y_{0}, scissor_w_{0}, scissor_h_{0};
	bool scissor_valid_{false};

	struct ColorTarget
	{
		SDL_GPUTexture* texture{nullptr};
		SDL_GPUTextureFormat format{SDL_GPU_TEXTUREFORMAT_INVALID};
		Uint32 mip_level{0};
		Uint32 layer{0};
	};
	std::vector<ColorTarget> pass_colors_;
	SDL_GPUTexture* pass_depth_{nullptr};
	SDL_GPUTextureFormat pass_depth_format_{SDL_GPU_TEXTUREFORMAT_INVALID};
	bool pass_has_depth_{false};

	static constexpr size_t kMaxPipelineColorTargets = 8;

	struct PipelineKey
	{
		SDL_GPUShader* vs{nullptr};
		SDL_GPUShader* ps{nullptr};
		void const* layout{nullptr};
		void const* state{nullptr};
		RenderLayout::topology_type topo{RenderLayout::TT_TriangleList};
		uint32_t num_colors{0};
		std::array<SDL_GPUTextureFormat, kMaxPipelineColorTargets> color_fmts{};
		bool has_depth{false};
		SDL_GPUTextureFormat depth_fmt{SDL_GPU_TEXTUREFORMAT_INVALID};

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
	std::unordered_map<PipelineKey, SDL_GPUGraphicsPipeline*, PipelineKeyHash> pipelines_;
	std::unique_ptr<SDL3MvpTriangle> mvp_triangle_;
	std::unique_ptr<SDL3SkyBoxPresent> skybox_present_;
	std::unique_ptr<SDL3MeshPresent> mesh_present_;
};

} // namespace RenderWorker
