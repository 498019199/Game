#pragma once

#include <render/RenderEngine.h>
#include "MetalUtil.h"
#include <math/color.h>
#include <array>
#include <memory>
#include <unordered_map>
#include <vector>

namespace RenderWorker
{

class MetalShaderStageObject;

struct MetalDeviceLifetime final
{
	metal::Device* device{nullptr};
};

class ZENGINE_METAL_API MetalRenderEngine : public RenderEngine
{
public:
	MetalRenderEngine();
	~MetalRenderEngine() override;

	bool RequiresFlipping() const override
	{
		return false;
	}

	void BeginFrame() override;
	void EndFrame() override;
	void BeginOverlayPass() override;
	bool CaptureCurrentColor(TexturePtr& snapshot) override;
	void ScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

	metal::Device* Device() const noexcept
	{
		return device_;
	}
	std::shared_ptr<MetalDeviceLifetime> const& DeviceLifetime() const noexcept
	{
		return device_lifetime_;
	}
	void* Window() const noexcept
	{
		return window_;
	}
	metal::CommandBuffer* CurrentCommandBuffer() const noexcept
	{
		return cmd_;
	}

	void Device(metal::Device* device, void* window);
	void DetachWindow() noexcept
	{
		window_ = nullptr;
	}
	void FillRenderDeviceCaps();

	void CurrentCommandBuffer(metal::CommandBuffer* cmd) noexcept
	{
		cmd_ = cmd;
	}
	void CurrentSwapchainTexture(metal::Texture* tex) noexcept
	{
		swapchain_tex_ = tex;
	}
	metal::Texture* CurrentSwapchainTexture() const noexcept
	{
		return swapchain_tex_;
	}

	metal::RenderPass* CurrentRenderPass() const noexcept
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

	metal::GraphicsPipeline* GetOrCreatePipeline(const RenderEffect& effect, const RenderPass& pass,
		const RenderLayout& rl);
	void EnsureDefaultSampleBinding();
    metal::Texture* DefaultTexture(metal::TextureType type);
	void BindStageSamplers(RenderEffect const& effect, MetalShaderStageObject const& stage, bool fragment);
	void ResolvePassTargets();
	void ApplyViewportAndScissor();

private:
	std::shared_ptr<MetalDeviceLifetime> device_lifetime_{std::make_shared<MetalDeviceLifetime>()};
	metal::Device* device_{nullptr};
	void* window_{nullptr};
	metal::CommandBuffer* cmd_{nullptr};
	metal::Texture* swapchain_tex_{nullptr};
	metal::RenderPass* render_pass_{nullptr};
	metal::Sampler* default_sampler_{nullptr};
	std::array<metal::Texture*, metal::TEXTURETYPE_CUBE_ARRAY + 1> default_textures_{};

	uint32_t scissor_x_{0}, scissor_y_{0}, scissor_w_{0}, scissor_h_{0};
	bool scissor_valid_{false};
	bool scissor_enabled_{false};

	struct ColorTarget
	{
		metal::Texture* texture{nullptr};
		metal::TextureFormat format{metal::TEXTUREFORMAT_INVALID};
		uint32_t mip_level{0};
		uint32_t layer{0};
	};
	std::vector<ColorTarget> pass_colors_;
	metal::Texture* pass_depth_{nullptr};
	metal::TextureFormat pass_depth_format_{metal::TEXTUREFORMAT_INVALID};
	bool pass_has_depth_{false};

	static constexpr size_t kMaxPipelineColorTargets = 8;

	struct PipelineKey
	{
		metal::Shader* vs{nullptr};
		metal::Shader* ps{nullptr};
		void const* layout{nullptr};
		void const* state{nullptr};
		RenderLayout::topology_type topo{RenderLayout::TT_TriangleList};
		uint32_t num_colors{0};
		std::array<metal::TextureFormat, kMaxPipelineColorTargets> color_fmts{};
		bool has_depth{false};
		metal::TextureFormat depth_fmt{metal::TEXTUREFORMAT_INVALID};

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
	std::unordered_map<PipelineKey, metal::GraphicsPipeline*, PipelineKeyHash> pipelines_;
};

} // namespace RenderWorker
