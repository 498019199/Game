#pragma once
#include <base/ZEngine.h>
#include <base/SmartPtrHelper.h>
#include <common/DllLoader.h>
#include <render/RenderLayout.h>
#include <render/RenderEngine.h>

#include "D3D11Util.h"
#include "D3D11AdapterList.h"

namespace RenderWorker
{

class D3D11RenderEngine :public RenderEngine
{
public:
    D3D11RenderEngine();
    ~D3D11RenderEngine();

#if ZENGINE_IS_DEV_PLATFORM
    void* GetD3DDevice() override;
    void* GetD3DDeviceImmContext() override;
#endif //ZENGINE_IS_DEV_PLATFORM

    void BeginFrame() override;
    void EndFrame() override;

    IDXGIFactory2* DXGIFactory2() const noexcept;
    IDXGIFactory3* DXGIFactory3() const noexcept;
    IDXGIFactory4* DXGIFactory4() const noexcept;
    IDXGIFactory5* DXGIFactory5() const noexcept;
    IDXGIFactory6* DXGIFactory6() const noexcept;
    uint8_t DXGISubVer() const noexcept;

    ID3D11Device1* D3DDevice1() const noexcept;
    ID3D11Device2* D3DDevice2() const noexcept;
    ID3D11Device3* D3DDevice3() const noexcept;
    ID3D11Device4* D3DDevice4() const noexcept;
    ID3D11Device5* D3DDevice5() const noexcept;
    ID3D11DeviceContext1* D3DDeviceImmContext1() const noexcept;
    ID3D11DeviceContext2* D3DDeviceImmContext2() const noexcept;
    ID3D11DeviceContext3* D3DDeviceImmContext3() const noexcept;
    ID3D11DeviceContext4* D3DDeviceImmContext4() const noexcept;
    uint8_t D3D11RuntimeSubVer() const noexcept;

	D3D_FEATURE_LEVEL DeviceFeatureLevel() const noexcept;

	void D3DDevice(ID3D11Device1* device, ID3D11DeviceContext1* imm_ctx, D3D_FEATURE_LEVEL feature_level);

    // 填充设备能力
    void FillRenderDeviceCaps();
        
    void DetectD3D11Runtime(ID3D11Device1* device, ID3D11DeviceContext1* imm_ctx);

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
    void RSSetViewports(UINT NumViewports, D3D11_VIEWPORT const * pViewports);
    void OMSetRenderTargets(UINT num_rtvs, ID3D11RenderTargetView* const * rtvs, ID3D11DepthStencilView* dsv);
    void OMSetRenderTargetsAndUnorderedAccessViews(UINT num_rtvs, ID3D11RenderTargetView* const * rtvs,
        ID3D11DepthStencilView* dsv, 
        UINT uav_start_slot, UINT num_uavs, ID3D11UnorderedAccessView* const * uavs, UINT const * uav_init_counts);
    void CSSetUnorderedAccessViews(UINT start_slot, UINT num_uavs, ID3D11UnorderedAccessView* const * uavs,
        UINT const * uav_init_counts);

    char const * DefaultShaderProfile(ShaderStage stage) const;

    // // 删除shader资源
    void DetachSRV(void* rtv_src, uint32_t rt_first_subres, uint32_t rt_num_subres);

	void ResetRenderStates();
    // 获取D3D适配器列表
    const D3D11AdapterList& D3DAdapters() const noexcept;
    // 获取当前适配器
	D3D11Adapter& ActiveAdapter() const;

    HRESULT D3D11CreateDevice(IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags,
        D3D_FEATURE_LEVEL const* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, ID3D11Device** ppDevice,
        D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext) const;

	static void CALLBACK OnDeviceLost(PTP_CALLBACK_INSTANCE instance, PVOID context, PTP_WAIT wait, TP_WAIT_RESULT wait_result) noexcept;
private:
    // 建立渲染窗口
	virtual void DoCreateRenderWindow(std::string const & name, RenderSettings const & settings) override;
    // 渲染
    void DoRender(const RenderEffect& effect, const RenderTechnique& tech, const RenderLayout& rl) override;
    // 设置当前渲染目标
	void DoBindFrameBuffer(FrameBufferPtr const & fb) override;
    // 设置当前Stream output目标
    virtual void DoBindSOBuffers(const RenderLayoutPtr& rl) override;

    virtual void DoDestroy() override;

private:
	typedef HRESULT(WINAPI *CreateDXGIFactory1Func)(REFIID riid, void** ppFactory);
    typedef HRESULT(WINAPI *CreateDXGIFactory2Func)(UINT flags, REFIID riid, void** ppFactory);
    typedef HRESULT(WINAPI *D3D11CreateDeviceFunc)(IDXGIAdapter* pAdapter,
        D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags,
        D3D_FEATURE_LEVEL const * pFeatureLevels, UINT FeatureLevels, UINT SDKVersion,
        ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext);

    CreateDXGIFactory1Func DynamicCreateDXGIFactory1_;
    CreateDXGIFactory2Func DynamicCreateDXGIFactory2_;
    D3D11CreateDeviceFunc DynamicD3D11CreateDevice_;

#ifdef ZENGINE_PLATFORM_WINDOWS_DESKTOP
    CommonWorker::DllLoader mod_dxgi_;
	CommonWorker::DllLoader mod_d3d11_;
#endif

    IDXGIFactory2Ptr gi_factory_2_;
	IDXGIFactory3Ptr gi_factory_3_;
    IDXGIFactory4Ptr gi_factory_4_;
    IDXGIFactory5Ptr gi_factory_5_;
    IDXGIFactory6Ptr gi_factory_6_;
    uint8_t dxgi_sub_ver_;

    ID3D11Device1Ptr d3d_device_1_;
    ID3D11Device2Ptr d3d_device_2_;
    ID3D11Device3Ptr d3d_device_3_;
    ID3D11Device4Ptr d3d_device_4_;
    ID3D11Device5Ptr d3d_device_5_;
    ID3D11DeviceContext1Ptr d3d_imm_ctx_1_;
    ID3D11DeviceContext2Ptr d3d_imm_ctx_2_;
    ID3D11DeviceContext3Ptr d3d_imm_ctx_3_;
    ID3D11DeviceContext4Ptr d3d_imm_ctx_4_;
    uint8_t d3d_11_runtime_sub_ver_;

	D3D_FEATURE_LEVEL d3d_feature_level_;

    // List of D3D drivers installed (video cards)
	// Enumerates itself
	D3D11AdapterList adapterList_;
    
    uint32_t num_primitives_just_rendered_{0};
	uint32_t num_vertices_just_rendered_{0};
    // 光栅状态
    ID3D11RasterizerState* rasterizer_state_cache_{nullptr};
    // 模板/深度状态
    ID3D11DepthStencilState* depth_stencil_state_cache_{nullptr};
    // 混合状态
    ID3D11BlendState* blend_state_cache_{nullptr};
    Color blend_factor_cache_{1, 1, 1, 1};
    uint32_t sample_mask_cache_{0xFFFFFFFF};
    uint16_t stencil_ref_cache_{0};
	D3D11_VIEWPORT viewport_cache_{};
    
    // 当前绑定的着色器
    ID3D11VertexShader* vertex_shader_cache_{nullptr};
    ID3D11PixelShader* pixel_shader_cache_{nullptr};
    ID3D11GeometryShader* geometry_shader_cache_{nullptr};

    std::array<std::vector<ID3D11Buffer*>, ShaderStageNum> shader_cb_ptr_cache_;
    std::array<std::vector<std::tuple<void*, uint32_t, uint32_t>>, ShaderStageNum> shader_srvsrc_cache_;
    // shader 图片绑定
    std::array<std::vector<ID3D11ShaderResourceView*>, ShaderStageNum> shader_srv_ptr_cache_;
    std::array<std::vector<ID3D11SamplerState*>, ShaderStageNum> shader_sampler_ptr_cache_;
	std::vector<ID3D11UnorderedAccessView*> render_uav_ptr_cache_;
	std::vector<uint32_t> render_uav_init_count_cache_;
	std::vector<ID3D11UnorderedAccessView*> compute_uav_ptr_cache_;
	std::vector<uint32_t> compute_uav_init_count_cache_;
	std::vector<ID3D11RenderTargetView*> rtv_ptr_cache_;
	ID3D11DepthStencilView* dsv_ptr_cache_{nullptr};

    uint32_t num_so_buffs_{0};
    // 顶点索引相关
    ID3D11InputLayout* input_layout_cache_{nullptr};
    std::vector<ID3D11Buffer*> vb_cache_;
    std::vector<UINT> vb_stride_cache_;
    std::vector<UINT> vb_offset_cache_;
    ID3D11Buffer* ib_cache_{nullptr};
    RenderLayout::topology_type topology_type_cache_ {RenderLayout::TT_PointList};
    
    // 默认shader 目标选项
    char const* shader_profiles_[ShaderStageNum];

    Win32UniqueHandle device_lost_event_;
    DWORD device_lost_reg_cookie_{0};
    Win32UniqueTpWait thread_pool_wait_;
};
}








