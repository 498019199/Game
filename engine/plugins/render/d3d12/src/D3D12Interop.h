#pragma once
#include "D3D12Gpu.h"
#include <d3d12.h>
namespace RenderWorker::d3d12
{
ZENGINE_D3D12_API ID3D12CommandQueue *NativeQueue(Device *);
ZENGINE_D3D12_API ID3D12DescriptorHeap *EditorHeap(Device *);
ZENGINE_D3D12_API void AllocateEditorView(Device *, D3D12_CPU_DESCRIPTOR_HANDLE &, D3D12_GPU_DESCRIPTOR_HANDLE &);
ZENGINE_D3D12_API void FreeEditorView(Device *, D3D12_CPU_DESCRIPTOR_HANDLE);
ZENGINE_D3D12_API void *EditorTexture(Texture *);
}
