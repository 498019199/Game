#pragma once
#include <base/Context.h>
#include <render/RenderLayout.h>
#include <render/RenderEngine.h>
#include "D3D11Util.h"

namespace RenderWorker
{
class D3D11RenderEngine :public RenderEngine
{
public:
    D3D11RenderEngine(HWND hwnd, const RenderSettings& settings);
    ~D3D11RenderEngine();

    void OnResize();

    ID3D11Device* D3DDevice() const;
    ID3D11DeviceContext* D3DDeviceImmContext() const;

    void BeginRender() const override;
    void DoRender(const RenderEffect& effect, const RenderTechnique& tech, const RenderLayout& rl) override;
    void EndRender() const override;
    void SwitchChain() const; 

    // 设置光栅化状态
    void RSSetState(ID3D11RasterizerState* ras);
    // 设置混合状态
    void OMSetBlendState(ID3D11BlendState* bs, const Color& blend_factor, uint32_t sample_mask);
    // 设置模板/深度
    void OMSetDepthStencilState(ID3D11DepthStencilState* ds, uint16_t stencil_ref);
    // 将着色器绑定到渲染管线
    void VSSetShader(ID3D11VertexShader* shader);
	void PSSetShader(ID3D11PixelShader* shader);
    void GSSetShader(ID3D11GeometryShader* shader);
    // 绑定shader资源
    void SetShaderResources(ShaderStage stage, std::span<std::tuple<void*, uint32_t, uint32_t> const> srvsrcs, std::span<ID3D11ShaderResourceView* const> srvs);
    // 绑定取样器
    void SetSamplers(ShaderStage stage, std::span<ID3D11SamplerState* const> samplers);
    // 将更新好的常量缓冲区绑定到顶点着色器和像素着色器
    void SetConstantBuffers(ShaderStage stage, std::span<ID3D11Buffer* const> cbs);

    char const * DefaultShaderProfile(ShaderStage stage) const;

    // // 删除shader资源
    void DetachSRV(void* rtv_src, uint32_t rt_first_subres, uint32_t rt_num_subres);
private:
    // 设置当前Stream output目标
    virtual void DoBindSOBuffers(const RenderLayoutPtr& rl) override;

    // 填充设备能力
    void FillRenderDeviceCaps();
private:
    int weight_{0};
    int height_{0};
    int sample_count_{0};
    int sample_quality_{0};
    
    ID3D11DevicePtr d3d_device_;
	ID3D11DeviceContextPtr d3d_imm_ctx_;
    D3D_FEATURE_LEVEL d3d_feature_level_;

    IDXGISwapChainPtr swap_chain_;
    
    uint32_t num_primitives_just_rendered_{0};
	uint32_t num_vertices_just_rendered_{0};

    ID3D11Texture2DPtr depth_stencil_buff_;
    ID3D11DepthStencilViewPtr depth_stencil_view_;

    ID3D11RenderTargetViewPtr render_target_view_;
    
    D3D11_VIEWPORT screen_viewport_;

    // 光栅状态
    ID3D11RasterizerState* rasterizer_state_cache_{nullptr};
    // 混合状态
    ID3D11BlendState* blend_state_cache_{nullptr};
    Color blend_factor_cache_{1, 1, 1, 1};
    uint32_t sample_mask_cache_{0xFFFFFFFF};
    // 模板/深度状态
    ID3D11DepthStencilState* depth_stencil_state_cache_{nullptr};
    uint16_t stencil_ref_cache_{0};
    
    // 当前绑定的着色器
    ID3D11VertexShader* vertex_shader_cache_{nullptr};
    ID3D11PixelShader* pixel_shader_cache_{nullptr};
    ID3D11GeometryShader* geometry_shader_cache_{nullptr};
    
    // 默认shader 目标选项
    char const* shader_profiles_[ShaderStageNum];

    std::array<std::vector<ID3D11Buffer*>, ShaderStageNum> shader_cb_ptr_cache_;
    std::array<std::vector<std::tuple<void*, uint32_t, uint32_t>>, ShaderStageNum> shader_srvsrc_cache_;
    // shader 图片绑定
    std::array<std::vector<ID3D11ShaderResourceView*>, ShaderStageNum> shader_srv_ptr_cache_;
    std::array<std::vector<ID3D11SamplerState*>, ShaderStageNum> shader_sampler_ptr_cache_;

    uint32_t num_so_buffs_{0};
    // 顶点索引相关
    ID3D11InputLayout* input_layout_cache_{nullptr};
    std::vector<ID3D11Buffer*> vb_cache_;
    std::vector<UINT> vb_stride_cache_;
    std::vector<UINT> vb_offset_cache_;
    ID3D11Buffer* ib_cache_{nullptr};
    RenderLayout::topology_type topology_type_cache_ {RenderLayout::TT_PointList};
};
}








