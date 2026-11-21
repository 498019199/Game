#pragma once
namespace RenderWorker
{
#ifdef _WIN32
inline constexpr char DesktopRendererName[] = "D3D12";
#else
inline constexpr char DesktopRendererName[] = "Metal";
#endif
}
