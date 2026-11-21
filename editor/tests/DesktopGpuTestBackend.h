#pragma once
#include <base/DesktopBackend.h>

#ifdef _WIN32
#include <D3D12RenderEngine.h>
#include <D3D12Texture.h>
namespace DesktopGpuTest {
using Renderer = RenderWorker::D3D12RenderEngine;
using Texture = RenderWorker::D3D12Texture;
namespace gpu = RenderWorker::d3d12;
}
#else
#include <MetalRenderEngine.h>
#include <MetalTexture.h>
namespace DesktopGpuTest {
using Renderer = RenderWorker::MetalRenderEngine;
using Texture = RenderWorker::MetalTexture;
namespace gpu = RenderWorker::metal;
}
#endif
