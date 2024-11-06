#pragma once
#include "D3D11AdapterList.h"
#include <render/IRenderDevice.h>
#include <core/IContext.h>

namespace CoreWorker
{

class D3D11RenderDevice: public IRenderDevice
{
public:
    D3D11RenderDevice(HWND hwnd, int width, int height);

    virtual void CreateRenderWindow(const std::string strName, const FWindowDesc& WindowDesc) override;
    virtual bool CreateDevice() override;
    virtual void Destroy() override;
    virtual void Refresh() override;

    void Render();

    D3D11Adapter* ActiveAdapter() const noexcept;

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
private:
    void DetectD3D11Runtime(ID3D11Device1* device, ID3D11DeviceContext1* imm_ctx);

    void FillRenderDeviceCaps();

    void DoCreateRenderWindow(const std::string strName, const FWindowDesc& WindowDesc);
private:
    HWND 			                    Hwnd_ = 0;
    int                                 nWidth_ = 0;
    int                                 Height_ = 0;

    typedef HRESULT(WINAPI *CreateDXGIFactory1Func)(REFIID riid, void** ppFactory);
    typedef HRESULT(WINAPI *CreateDXGIFactory2Func)(UINT flags, REFIID riid, void** ppFactory);
    typedef HRESULT(WINAPI *D3D11CreateDeviceFunc)(IDXGIAdapter* pAdapter,
        D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags,
        D3D_FEATURE_LEVEL const * pFeatureLevels, UINT FeatureLevels, UINT SDKVersion,
        ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext);

    CreateDXGIFactory1Func DynamicCreateDXGIFactory1_;
    CreateDXGIFactory2Func DynamicCreateDXGIFactory2_;
    D3D11CreateDeviceFunc DynamicD3D11CreateDevice_;
#if defined(DEMOENGINE_PLATFORM_WINDOWS) && defined(DEMOENGINE_PLATFORM_WINDOWS_DESKTOP)
	void* mod_dxgi_ = nullptr;
    void* mod_d3d11_ = nullptr;
#endif

    IDXGIFactory2* gi_factory_2_ = nullptr;
    IDXGIFactory3* gi_factory_3_ = nullptr;
    IDXGIFactory4* gi_factory_4_ = nullptr;
    IDXGIFactory5* gi_factory_5_ = nullptr;
    IDXGIFactory6* gi_factory_6_ = nullptr;
    uint8_t dxgi_sub_ver_ = 0;

    ID3D11Device1* d3d_device_1_ = nullptr;
    ID3D11Device2* d3d_device_2_ = nullptr;
    ID3D11Device3* d3d_device_3_ = nullptr;
    ID3D11Device4* d3d_device_4_ = nullptr;
    ID3D11Device5* d3d_device_5_ = nullptr;
    ID3D11DeviceContext1* d3d_imm_ctx_1_ = nullptr;
    ID3D11DeviceContext2* d3d_imm_ctx_2_ = nullptr;
    ID3D11DeviceContext3* d3d_imm_ctx_3_ = nullptr;
    ID3D11DeviceContext4* d3d_imm_ctx_4_ = nullptr;
    uint8_t d3d_11_runtime_sub_ver_ = 0;

    D3D_FEATURE_LEVEL d3d_feature_level_;

    // List of D3D drivers installed (video cards)
	// Enumerates itself
	D3D11AdapterList adapterList_;


    ID3D11VertexShader* vertex_shader_cache_{nullptr};
    ID3D11PixelShader* pixel_shader_cache_{nullptr};
    ID3D11GeometryShader* geometry_shader_cache_{nullptr};
    ID3D11ComputeShader* compute_shader_cache_{nullptr};
    ID3D11HullShader* hull_shader_cache_{nullptr};
    ID3D11DomainShader* domain_shader_cache_{nullptr};
};
}
