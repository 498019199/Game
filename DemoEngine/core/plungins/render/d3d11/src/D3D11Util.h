#pragma once
#include <dxgi1_6.h>
#include <d3d11_4.h>
#include <common/com_ptr.h>
#include <common/macro.h>
#include <common/common.h>

#include <render/ElementFormat.h>
#include <render/RenderLayout.h>
#include <render/Texture.h>
#include <render/ShaderObject.h>
#include <render/RenderStateObject.h>

namespace RenderWorker
{
    using IDXGIFactory2Ptr = CommonWorker::com_ptr<IDXGIFactory2>;
    using IDXGIFactory3Ptr = CommonWorker::com_ptr<IDXGIFactory3>;
    using IDXGIFactory4Ptr = CommonWorker::com_ptr<IDXGIFactory4>;
    using IDXGIFactory5Ptr = CommonWorker::com_ptr<IDXGIFactory5>;
    using IDXGIFactory6Ptr = CommonWorker::com_ptr<IDXGIFactory6>;
	using IDXGIAdapter2Ptr = CommonWorker::com_ptr<IDXGIAdapter2>;
    using ID3D11Device1Ptr = CommonWorker::com_ptr<ID3D11Device1>;
    using ID3D11Device2Ptr = CommonWorker::com_ptr<ID3D11Device2>;
    using ID3D11Device3Ptr = CommonWorker::com_ptr<ID3D11Device3>;
    using ID3D11Device4Ptr = CommonWorker::com_ptr<ID3D11Device4>;
    using ID3D11Device5Ptr = CommonWorker::com_ptr<ID3D11Device5>;
    using ID3D11DeviceContext1Ptr = CommonWorker::com_ptr<ID3D11DeviceContext1>;
    using ID3D11DeviceContext2Ptr = CommonWorker::com_ptr<ID3D11DeviceContext2>;
    using ID3D11DeviceContext3Ptr = CommonWorker::com_ptr<ID3D11DeviceContext3>;
    using ID3D11DeviceContext4Ptr = CommonWorker::com_ptr<ID3D11DeviceContext4>;
    using IDXGISwapChain1Ptr = CommonWorker::com_ptr<IDXGISwapChain1>;
    using IDXGISwapChain2Ptr = CommonWorker::com_ptr<IDXGISwapChain2>;
    using IDXGISwapChain3Ptr = CommonWorker::com_ptr<IDXGISwapChain3>;
    using IDXGISwapChain4Ptr = CommonWorker::com_ptr<IDXGISwapChain4>;
    using ID3D11BufferPtr = CommonWorker::com_ptr<ID3D11Buffer>;
    using ID3D11InputLayoutPtr = CommonWorker::com_ptr<ID3D11InputLayout>;
    using ID3D11VertexShaderPtr = CommonWorker::com_ptr<ID3D11VertexShader>;
    using ID3D11PixelShaderPtr = CommonWorker::com_ptr<ID3D11PixelShader>;
    using ID3D11GeometryShaderPtr = CommonWorker::com_ptr<ID3D11GeometryShader>;
	using ID3D11ResourcePtr = CommonWorker::com_ptr<ID3D11Resource>;
	using ID3D11Texture1DPtr = CommonWorker::com_ptr<ID3D11Texture1D>;
	using ID3D11Texture2DPtr = CommonWorker::com_ptr<ID3D11Texture2D>;
	using ID3D11Texture3DPtr = CommonWorker::com_ptr<ID3D11Texture3D>;
	using ID3D11TextureCubePtr = CommonWorker::com_ptr<ID3D11Texture2D>;
    using ID3D11RenderTargetViewPtr = CommonWorker::com_ptr<ID3D11RenderTargetView>;
	using ID3D11DepthStencilViewPtr = CommonWorker::com_ptr<ID3D11DepthStencilView>;
    using ID3D11InputLayoutPtr = CommonWorker::com_ptr<ID3D11InputLayout>;
    using ID3D11SamplerStatePtr = CommonWorker::com_ptr<ID3D11SamplerState>;
	using ID3D11ShaderResourceViewPtr = CommonWorker::com_ptr<ID3D11ShaderResourceView>;
    using ID3D11RasterizerStatePtr = CommonWorker::com_ptr<ID3D11RasterizerState>;
    using ID3D11BlendStatePtr = CommonWorker::com_ptr<ID3D11BlendState>;
    using ID3D11DepthStencilStatePtr = CommonWorker::com_ptr<ID3D11DepthStencilState>;
}

namespace RenderWorker
{

class D3D11Mapping final
{
public:
    static DXGI_FORMAT MappingFormat(ElementFormat format);
    static ElementFormat MappingFormat(DXGI_FORMAT d3dfmt);

    static D3D11_FILTER Mapping(TexFilterOp filter);
    static D3D11_TEXTURE_ADDRESS_MODE Mapping(TexAddressingMode mode);
    static D3D11_COMPARISON_FUNC Mapping(CompareFunction func);

    static D3D11_CULL_MODE Mapping(CullMode mode);
    static D3D11_FILL_MODE Mapping(PolygonMode mode);
    
    static D3D11_BLEND Mapping(AlphaBlendFactor factor);
    static D3D11_BLEND_OP Mapping(BlendOperation bo);
    static uint32_t MappingColorMask(uint32_t mask);

    static D3D11_DEPTH_WRITE_MASK Mapping(bool depth_write_mask);
    static D3D11_STENCIL_OP Mapping(StencilOperation op);
    
    static D3D11_PRIMITIVE_TOPOLOGY Mapping(RenderLayout::topology_type tt);
    static void Mapping(std::vector<D3D11_INPUT_ELEMENT_DESC>& elements, uint32_t stream, std::span<const VertexElement> vet,
        RenderLayout::stream_type type, uint32_t freq);

    static D3D11_SO_DECLARATION_ENTRY Mapping(const ShaderDesc::StreamOutputDecl& decl);
};
}