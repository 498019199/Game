#pragma once
#include <d3d11_1.h>
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
    using ID3D11DevicePtr = com_ptr<ID3D11Device>;
    using ID3D11DeviceContextPtr = com_ptr<ID3D11DeviceContext>;
    using IDXGISwapChainPtr = com_ptr<IDXGISwapChain>;
    using ID3D11BufferPtr = com_ptr<ID3D11Buffer>;
    using ID3D11InputLayoutPtr = com_ptr<ID3D11InputLayout>;

    using ID3D11VertexShaderPtr = com_ptr<ID3D11VertexShader>;
    using ID3D11PixelShaderPtr = com_ptr<ID3D11PixelShader>;
    using ID3D11GeometryShaderPtr = com_ptr<ID3D11GeometryShader>;
	using ID3D11ResourcePtr = com_ptr<ID3D11Resource>;
	using ID3D11Texture1DPtr = com_ptr<ID3D11Texture1D>;
	using ID3D11Texture2DPtr = com_ptr<ID3D11Texture2D>;
	using ID3D11Texture3DPtr = com_ptr<ID3D11Texture3D>;
	using ID3D11TextureCubePtr = com_ptr<ID3D11Texture2D>;
    using ID3D11RenderTargetViewPtr = com_ptr<ID3D11RenderTargetView>;
	using ID3D11DepthStencilViewPtr = com_ptr<ID3D11DepthStencilView>;
    using ID3D11InputLayoutPtr = com_ptr<ID3D11InputLayout>;
    using ID3D11SamplerStatePtr = com_ptr<ID3D11SamplerState>;
	using ID3D11ShaderResourceViewPtr = com_ptr<ID3D11ShaderResourceView>;
    using ID3D11RasterizerStatePtr = com_ptr<ID3D11RasterizerState>;
    using ID3D11BlendStatePtr = com_ptr<ID3D11BlendState>;
    using ID3D11DepthStencilStatePtr = com_ptr<ID3D11DepthStencilState>;
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