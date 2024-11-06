#include "D3D11Adapter.h"
#include <algorithm>

namespace CoreWorker
{
bool D3D11VideoMode::operator<(D3D11VideoMode const & rhs) const noexcept
{
    if (width_ < rhs.width_)
    {
        return true;
    }
    else if (width_ == rhs.width_)
    {
        if (height_ < rhs.height_)
        {
            return true;
        }
        else if (height_ == rhs.height_)
        {
            if (format_ < rhs.format_)
            {
                return true;
            }
        }
    }

    return false;
}

bool D3D11VideoMode::operator==(D3D11VideoMode const & rhs) const noexcept
{
    return (width_ == rhs.width_) && (height_ == rhs.height_) && (format_ == rhs.format_);
}

D3D11Adapter::D3D11Adapter(uint32_t adapter_no, IDXGIAdapter2* adapter)
    :adapter_no_(adapter_no)
{
    ResetAdapter(adapter);
}

void D3D11Adapter::Enumerate()
{
    static DXGI_FORMAT constexpr formats[] =
    {
        DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
        DXGI_FORMAT_B8G8R8A8_UNORM,
        DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
        DXGI_FORMAT_R10G10B10A2_UNORM
    };

    UINT i = 0;
    IDXGIOutput* output;
    while (adapter_->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
    {
        if (output != nullptr)
        {
            for (auto const & format : formats)
            {
                UINT num = 0;
				output->GetDisplayModeList(format, DXGI_ENUM_MODES_SCALING, &num, nullptr);
                if (num > 0)
				{
                    std::vector<DXGI_MODE_DESC> mode_descs(num);
					output->GetDisplayModeList(format, DXGI_ENUM_MODES_SCALING, &num, &mode_descs[0]);
					for (auto const & mode_desc : mode_descs)
					{
                        D3D11VideoMode video_mode(mode_desc.Width, mode_desc.Height, mode_desc.Format);
                        // 如果找到一个新模式, 加入模式列表
                        if (std::find(modes_.begin(), modes_.end(), video_mode) == modes_.end())
                        {
                            modes_.emplace_back(std::move(video_mode));
                        }
                    }
                }
            }
        }

        ++ i;
    }
    std::sort(modes_.begin(), modes_.end());
}

// 获取设备描述字符串
/////////////////////////////////////////////////////////////////////////////////
std::wstring const D3D11Adapter::Description() const
{
    return std::wstring(adapter_desc_.Description);
}

void D3D11Adapter::ResetAdapter(IDXGIAdapter2* adapter)
{
    adapter_ = adapter;
    adapter_->GetDesc2(&adapter_desc_);
}


}