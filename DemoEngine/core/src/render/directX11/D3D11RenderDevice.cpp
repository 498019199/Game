#include <core/IContext.h>
#include <render/ElementFormat.h>
#include "D3D11RenderDevice.h"
#include "D3D11RenderWindow.h"
#include <cassert>

#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }      //自定义一个SAFE_RELEASE()宏,便于COM资源的释放 
#define GET_FUNCTION(mod, func_type, func_name) reinterpret_cast<func_type>(GetProcAddress(static_cast<HMODULE>(mod), func_name));

namespace CoreWorker
{

D3D11RenderDevice::D3D11RenderDevice(HWND hwnd, int width, int height)
	:Hwnd_(hwnd), nWidth_(width), Height_(height)
{
#ifdef DEMOENGINE_PLATFORM_WINDOWS
	mod_dxgi_ = static_cast<void*>(::LoadLibraryExA("dxgi.dll", nullptr, 0));
	if(nullptr == mod_dxgi_)
	{
		LOGER_ERROR() << "COULDN'T load dxgi.dll";
		return ;
	}
	mod_d3d11_ = static_cast<void*>(::LoadLibraryExA("d3d11.dll", nullptr, 0));
	if(nullptr == mod_d3d11_)
	{
		LOGER_ERROR() << "COULDN'T load d3d11.dll";
		return ;
	}
	DynamicCreateDXGIFactory1_ = GET_FUNCTION(mod_dxgi_, CreateDXGIFactory1Func, "CreateDXGIFactory1");
	DynamicCreateDXGIFactory2_ = GET_FUNCTION(mod_dxgi_, CreateDXGIFactory2Func, "CreateDXGIFactory2");
	DynamicD3D11CreateDevice_ = GET_FUNCTION(mod_d3d11_, D3D11CreateDeviceFunc, "D3D11CreateDevice");
#else
	DynamicCreateDXGIFactory1_ = ::CreateDXGIFactory1;
	DynamicCreateDXGIFactory2_ = ::CreateDXGIFactory2;
	DynamicD3D11CreateDevice_ = ::D3D11CreateDevice;
#endif//

    HRESULT hr = E_FAIL;
	if(DynamicCreateDXGIFactory2_)
	{
		UINT const dxgi_factory_flags = 0;
		static UINT const available_dxgi_factory_flags[] =
		{
#ifdef _DEBUG
			dxgi_factory_flags | DXGI_CREATE_FACTORY_DEBUG,
#endif
			dxgi_factory_flags
		};
		for (auto const& flags : available_dxgi_factory_flags)
		{
			hr = DynamicCreateDXGIFactory2_(flags, __uuidof(IDXGIFactory2), PTR_PUT_VOID(gi_factory_2_));
			if (SUCCEEDED(hr))
			{
				break;
			}
		}
	}
	else
	{
		hr = DynamicCreateDXGIFactory1_(__uuidof(IDXGIFactory2), PTR_PUT_VOID(gi_factory_2_));
	}

	dxgi_sub_ver_ = 2;
	if (SUCCEEDED(gi_factory_2_->QueryInterface(&gi_factory_3_)))
	{
		dxgi_sub_ver_ = 3;
		if (SUCCEEDED(gi_factory_2_->QueryInterface(&gi_factory_4_)))
		{
			dxgi_sub_ver_ = 4;
			if (SUCCEEDED(gi_factory_2_->QueryInterface(&gi_factory_5_)))
			{
				dxgi_sub_ver_ = 5;
				if (SUCCEEDED(gi_factory_2_->QueryInterface(&gi_factory_6_)))
				{
					dxgi_sub_ver_ = 6;
				}
			}
		}
	}
	if (gi_factory_6_)
	{
		adapterList_.Enumerate(gi_factory_6_);
	}
	else
	{
		adapterList_.Enumerate(gi_factory_2_);
	}
}

bool D3D11RenderDevice::CreateDevice()
{
	HRESULT hr = E_FAIL;
	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    ID3D11Device1* d3d_device = nullptr;
    ID3D11DeviceContext1* d3d_imm_ctx = nullptr;

	ID3D11Device* this_device;
	ID3D11DeviceContext* this_imm_ctx;
	D3D_FEATURE_LEVEL this_out_feature_level;
    D3D_DRIVER_TYPE dev_type = D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_UNKNOWN;
    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
        dev_type = driverTypes[driverTypeIndex];
        hr = DynamicD3D11CreateDevice_(0, dev_type, nullptr, createDeviceFlags, 
            featureLevels, numFeatureLevels, D3D11_SDK_VERSION, &this_device, &this_out_feature_level, &this_imm_ctx);
        if (SUCCEEDED(hr))
        {
            this_device->QueryInterface(&d3d_device);
            this_imm_ctx->QueryInterface(&d3d_imm_ctx);
            d3d_feature_level_ = this_out_feature_level;
            break;
        }
    }
    if (SUCCEEDED(hr))
    {
        DetectD3D11Runtime(d3d_device, d3d_imm_ctx);
        FillRenderDeviceCaps();
    }
    return true;
}

void D3D11RenderDevice::Render()
{
	int num_rtvs = 0;
	ID3D11RenderTargetView *const * rtvs = nullptr;
	ID3D11DepthStencilView *dsv = nullptr;
	// 前用于渲染的渲染目标（Render Targets）。
	d3d_imm_ctx_1_->OMSetRenderTargets(num_rtvs, rtvs, dsv);
}

void D3D11RenderDevice::DetectD3D11Runtime(ID3D11Device1* device, ID3D11DeviceContext1* imm_ctx)
{
    d3d_device_1_ = device;
    d3d_imm_ctx_1_ = imm_ctx;
    d3d_11_runtime_sub_ver_ = 1;

    if(SUCCEEDED(device->QueryInterface(&d3d_device_2_)) && SUCCEEDED(imm_ctx->QueryInterface(&d3d_imm_ctx_2_)))
    {
        d3d_11_runtime_sub_ver_ = 2;
        if(SUCCEEDED(device->QueryInterface(&d3d_device_3_)) && SUCCEEDED(imm_ctx->QueryInterface(&d3d_imm_ctx_3_)))
        {
            d3d_11_runtime_sub_ver_ = 3;
            if(SUCCEEDED(device->QueryInterface(&d3d_device_4_)))
            {
                d3d_11_runtime_sub_ver_ = 4;
                d3d_device_1_->QueryInterface(&d3d_device_5_);
                d3d_imm_ctx_1_->QueryInterface(&d3d_imm_ctx_4_);
            }
        }
    }
}

// 填充设备能力
void D3D11RenderDevice::FillRenderDeviceCaps()
{
    COMMON_ASSERT(d3d_device_1_);
    switch (d3d_feature_level_)
    {
        case D3D_FEATURE_LEVEL_12_1:
		case D3D_FEATURE_LEVEL_12_0:
		case D3D_FEATURE_LEVEL_11_1:
		case D3D_FEATURE_LEVEL_11_0:
        // D3D11 feature level 12.1+ supports objects in shader model 5.1, although it doesn't support shader model 5.1 bytecode
			
            break;
        
		default:
			LOGER_ERROR() << "Invalid feature level";
    }

    std::pair<ElementFormat, DXGI_FORMAT> const fmts[] = 
    {
        std::make_pair(EF_A8, DXGI_FORMAT_A8_UNORM),
        std::make_pair(EF_R5G6B5, DXGI_FORMAT_B5G6R5_UNORM),
        std::make_pair(EF_A1RGB5, DXGI_FORMAT_B5G5R5A1_UNORM),
        std::make_pair(EF_ARGB4, DXGI_FORMAT_B4G4R4A4_UNORM),
        std::make_pair(EF_R8, DXGI_FORMAT_R8_UNORM),
        std::make_pair(EF_SIGNED_R8, DXGI_FORMAT_R8_SNORM),
        std::make_pair(EF_GR8, DXGI_FORMAT_R8G8_UNORM),
        std::make_pair(EF_SIGNED_GR8, DXGI_FORMAT_R8G8_SNORM),
        std::make_pair(EF_ARGB8, DXGI_FORMAT_B8G8R8A8_UNORM),
        std::make_pair(EF_ABGR8, DXGI_FORMAT_R8G8B8A8_UNORM),
        std::make_pair(EF_SIGNED_ABGR8, DXGI_FORMAT_R8G8B8A8_SNORM),
        std::make_pair(EF_A2BGR10, DXGI_FORMAT_R10G10B10A2_UNORM),
        std::make_pair(EF_SIGNED_A2BGR10, DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM),
        std::make_pair(EF_R8UI, DXGI_FORMAT_R8_UINT),
        std::make_pair(EF_R8I, DXGI_FORMAT_R8_SINT),
        std::make_pair(EF_GR8UI, DXGI_FORMAT_R8G8_UINT),
        std::make_pair(EF_GR8I, DXGI_FORMAT_R8G8_SINT),
        std::make_pair(EF_ABGR8UI, DXGI_FORMAT_R8G8B8A8_UINT),
        std::make_pair(EF_ABGR8I, DXGI_FORMAT_R8G8B8A8_SINT),
        std::make_pair(EF_A2BGR10UI, DXGI_FORMAT_R10G10B10A2_UINT),
        std::make_pair(EF_R16, DXGI_FORMAT_R16_UNORM),
        std::make_pair(EF_SIGNED_R16, DXGI_FORMAT_R16_SNORM),
        std::make_pair(EF_GR16, DXGI_FORMAT_R16G16_UNORM),
        std::make_pair(EF_SIGNED_GR16, DXGI_FORMAT_R16G16_SNORM),
        std::make_pair(EF_ABGR16, DXGI_FORMAT_R16G16B16A16_UNORM),
        std::make_pair(EF_SIGNED_ABGR16, DXGI_FORMAT_R16G16B16A16_SNORM),
        std::make_pair(EF_R16UI, DXGI_FORMAT_R16_UINT),
        std::make_pair(EF_R16I, DXGI_FORMAT_R16_SINT),
        std::make_pair(EF_GR16UI, DXGI_FORMAT_R16G16_UINT),
        std::make_pair(EF_GR16I, DXGI_FORMAT_R16G16_SINT),
        std::make_pair(EF_ABGR16UI, DXGI_FORMAT_R16G16B16A16_UINT),
        std::make_pair(EF_ABGR16I, DXGI_FORMAT_R16G16B16A16_SINT),
        std::make_pair(EF_R32UI, DXGI_FORMAT_R32_UINT),
        std::make_pair(EF_R32I, DXGI_FORMAT_R32_SINT),
        std::make_pair(EF_GR32UI, DXGI_FORMAT_R32G32_UINT),
        std::make_pair(EF_GR32I, DXGI_FORMAT_R32G32_SINT),
        std::make_pair(EF_BGR32UI, DXGI_FORMAT_R32G32B32_UINT),
        std::make_pair(EF_BGR32I, DXGI_FORMAT_R32G32B32_SINT),
        std::make_pair(EF_ABGR32UI, DXGI_FORMAT_R32G32B32A32_UINT),
        std::make_pair(EF_ABGR32I, DXGI_FORMAT_R32G32B32A32_SINT),
        std::make_pair(EF_R16F, DXGI_FORMAT_R16_FLOAT),
        std::make_pair(EF_GR16F, DXGI_FORMAT_R16G16_FLOAT),
        std::make_pair(EF_B10G11R11F, DXGI_FORMAT_R11G11B10_FLOAT),
        std::make_pair(EF_ABGR16F, DXGI_FORMAT_R16G16B16A16_FLOAT),
        std::make_pair(EF_R32F, DXGI_FORMAT_R32_FLOAT),
        std::make_pair(EF_GR32F, DXGI_FORMAT_R32G32_FLOAT),
        std::make_pair(EF_BGR32F, DXGI_FORMAT_R32G32B32_FLOAT),
        std::make_pair(EF_ABGR32F, DXGI_FORMAT_R32G32B32A32_FLOAT),
        std::make_pair(EF_BC1, DXGI_FORMAT_BC1_UNORM),
        std::make_pair(EF_BC2, DXGI_FORMAT_BC2_UNORM),
        std::make_pair(EF_BC3, DXGI_FORMAT_BC3_UNORM),
        std::make_pair(EF_BC4, DXGI_FORMAT_BC4_UNORM),
        std::make_pair(EF_SIGNED_BC4, DXGI_FORMAT_BC4_SNORM),
        std::make_pair(EF_BC5, DXGI_FORMAT_BC5_UNORM),
        std::make_pair(EF_SIGNED_BC5, DXGI_FORMAT_BC5_SNORM),
        std::make_pair(EF_BC6, DXGI_FORMAT_BC6H_UF16),
        std::make_pair(EF_SIGNED_BC6, DXGI_FORMAT_BC6H_SF16),
        std::make_pair(EF_BC7, DXGI_FORMAT_BC7_UNORM),
        std::make_pair(EF_D16, DXGI_FORMAT_D16_UNORM),
        std::make_pair(EF_D24S8, DXGI_FORMAT_D24_UNORM_S8_UINT),
        std::make_pair(EF_D32F, DXGI_FORMAT_D32_FLOAT),
        std::make_pair(EF_ARGB8_SRGB, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB),
        std::make_pair(EF_ABGR8_SRGB, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB),
        std::make_pair(EF_BC1_SRGB, DXGI_FORMAT_BC1_UNORM_SRGB),
        std::make_pair(EF_BC2_SRGB, DXGI_FORMAT_BC2_UNORM_SRGB),
        std::make_pair(EF_BC3_SRGB, DXGI_FORMAT_BC3_UNORM_SRGB),
        std::make_pair(EF_BC7_SRGB, DXGI_FORMAT_BC7_UNORM_SRGB)
    };

	// 检查4X多重采样质量等级
	UINT quality;
	if (SUCCEEDED(d3d_device_1_->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &quality)))
	{
	}

}

D3D11Adapter* D3D11RenderDevice::ActiveAdapter() const noexcept
{
    return adapterList_.Adapter(adapterList_.CurrentAdapterIndex());
}

IDXGIFactory2* D3D11RenderDevice::DXGIFactory2() const noexcept
{
    return gi_factory_2_;
}

IDXGIFactory3* D3D11RenderDevice::DXGIFactory3() const noexcept
{
    return gi_factory_3_;
}

IDXGIFactory4* D3D11RenderDevice::DXGIFactory4() const noexcept
{
    return gi_factory_4_;
}

IDXGIFactory5* D3D11RenderDevice::DXGIFactory5() const noexcept
{
    return gi_factory_5_;
}

IDXGIFactory6* D3D11RenderDevice::DXGIFactory6() const noexcept
{
    return gi_factory_6_;
}

uint8_t D3D11RenderDevice:: DXGISubVer() const noexcept
{
    return dxgi_sub_ver_;
}

ID3D11Device1* D3D11RenderDevice::D3DDevice1() const noexcept
{
    return d3d_device_1_;
}

ID3D11Device2* D3D11RenderDevice::D3DDevice2() const noexcept
{
    return d3d_device_2_;
}

ID3D11Device3* D3D11RenderDevice::D3DDevice3() const noexcept
{
    return d3d_device_3_;
}

ID3D11Device4* D3D11RenderDevice::D3DDevice4() const noexcept
{
    return d3d_device_4_;
}

ID3D11Device5* D3D11RenderDevice::D3DDevice5() const noexcept
{
    return d3d_device_5_;
}

ID3D11DeviceContext1* D3D11RenderDevice::D3DDeviceImmContext1() const noexcept
{
    return d3d_imm_ctx_1_;
}

ID3D11DeviceContext2* D3D11RenderDevice::D3DDeviceImmContext2() const noexcept
{
    return d3d_imm_ctx_2_;
}

ID3D11DeviceContext3* D3D11RenderDevice::D3DDeviceImmContext3() const noexcept
{
    return d3d_imm_ctx_3_;
}

ID3D11DeviceContext4* D3D11RenderDevice::D3DDeviceImmContext4() const noexcept
{
    return d3d_imm_ctx_4_;
}

uint8_t D3D11RenderDevice::D3D11RuntimeSubVer() const noexcept
{
    return d3d_11_runtime_sub_ver_;
}

void D3D11RenderDevice::CreateRenderWindow(const std::string strName, const FWindowDesc& WindowDesc)
{
    DoCreateRenderWindow(strName, WindowDesc);
}

void D3D11RenderDevice::DoCreateRenderWindow(const std::string strName, const FWindowDesc& WindowDesc)
{
    auto win = CommonWorker::MakeSharedPtr<D3D11RenderWindow>(ActiveAdapter(), strName, WindowDesc);
}

void D3D11RenderDevice::Destroy()
{
    adapterList_.Destroy();

    if (d3d_imm_ctx_1_)
    {
        d3d_imm_ctx_1_->ClearState();
        d3d_imm_ctx_1_->Flush();
    }
    d3d_imm_ctx_1_->Release();
    d3d_imm_ctx_2_->Release();
    d3d_imm_ctx_3_->Release();
    d3d_imm_ctx_4_->Release();
    d3d_device_1_->Release();
    d3d_device_2_->Release();
    d3d_device_3_->Release();
    d3d_device_4_->Release();
    d3d_device_5_->Release();
    gi_factory_2_->Release();
    gi_factory_3_->Release();
    gi_factory_4_->Release();
    gi_factory_5_->Release();
    gi_factory_6_->Release();

    DynamicCreateDXGIFactory1_ = nullptr;
    DynamicCreateDXGIFactory2_ = nullptr;
    DynamicD3D11CreateDevice_ = nullptr;
#ifdef DEMOENGINE_PLATFORM_WINDOWS
	::FreeLibrary(static_cast<HMODULE>(mod_dxgi_));
	::FreeLibrary(static_cast<HMODULE>(mod_d3d11_));
#endif//DEMOENGINE_PLATFORM_WINDOWS
}

void D3D11RenderDevice::Refresh()
{
}

}