#include <common/Util.h>
#include <common/common.h>
#include <common/ResIdentifier.h>

#ifdef ZENGINE_PLATFORM_WINDOWS
	#include <windows.h>
#endif

#include <vector>
#include <algorithm>

namespace RenderWorker
{

// 把一个wstring转化为wstring
/////////////////////////////////////////////////////////////////////////////////
std::wstring& Convert(std::wstring& dest, std::wstring_view src)
{
    dest = std::wstring(src);

    return dest;
}

// 把一个string转化为wstring
/////////////////////////////////////////////////////////////////////////////////
std::wstring& Convert(std::wstring& dest, std::string_view src)
{
    int const wcs_len = MultiByteToWideChar(CP_ACP, 0, src.data(), static_cast<int>(src.size()), nullptr, 0);
    auto tmp = std::make_unique<wchar_t[]>(wcs_len + 1);
    MultiByteToWideChar(CP_ACP, 0, src.data(), static_cast<int>(src.size()), tmp.get(), wcs_len);
	dest.assign(&tmp[0], &tmp[wcs_len]);
	return dest;
}

// Endian的切换
/////////////////////////////////////////////////////////////////////////////////
template <>
void EndianSwitch<2>(void* p) noexcept
{
    uint8_t* bytes = static_cast<uint8_t*>(p);
    std::swap(bytes[0], bytes[1]);
}

template <>
void EndianSwitch<4>(void* p) noexcept
{
    uint8_t* bytes = static_cast<uint8_t*>(p);
    std::swap(bytes[0], bytes[3]);
    std::swap(bytes[1], bytes[2]);
}

template <>
void EndianSwitch<8>(void* p) noexcept
{
    uint8_t* bytes = static_cast<uint8_t*>(p);
    std::swap(bytes[0], bytes[7]);
    std::swap(bytes[1], bytes[6]);
    std::swap(bytes[2], bytes[5]);
    std::swap(bytes[3], bytes[4]);
}


std::string ReadShortString(ResIdentifier& res)
{
    uint8_t len = 0;
    res.read(&len, sizeof(len));

    std::string tmp;
    if (len > 0)
    {
        tmp.resize(len);
        res.read(&tmp[0], len * sizeof(tmp[0]));
    }

    return tmp;
}

void WriteShortString(std::ostream& os, std::string_view str)
{
    uint8_t len = static_cast<uint8_t>(std::min(str.size(), static_cast<size_t>(255)));
    os.write(reinterpret_cast<char*>(&len), sizeof(len));

    if (len > 0)
    {
        os.write(str.data(), len * sizeof(str[0]));
    }
}
}