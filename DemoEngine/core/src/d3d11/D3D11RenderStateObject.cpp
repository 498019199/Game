#include "D3D11RenderStateObject.h"
#include "D3D11RenderEngine.h"

#include <base/Context.h>

namespace RenderWorker
{
D3D11SamplerStateObject::D3D11SamplerStateObject(SamplerStateDesc const & desc)
    : SamplerStateObject(desc)
{
    const auto& d3d11_re = checked_cast<const D3D11RenderEngine&>(Context::Instance().RenderEngineInstance());
    ID3D11Device* d3d_device = d3d11_re.D3DDevice();

    D3D11_SAMPLER_DESC d3d_desc;
    d3d_desc.Filter = D3D11Mapping::Mapping(desc.filter);
    d3d_desc.AddressU = D3D11Mapping::Mapping(desc.addr_mode_u);
    d3d_desc.AddressV = D3D11Mapping::Mapping(desc.addr_mode_v);
    d3d_desc.AddressW = D3D11Mapping::Mapping(desc.addr_mode_w);
    d3d_desc.MipLODBias = desc.mip_map_lod_bias;
    d3d_desc.MaxAnisotropy = desc.max_anisotropy;
    d3d_desc.ComparisonFunc = D3D11Mapping::Mapping(desc.cmp_func);
    d3d_desc.BorderColor[0] = desc.border_clr.r();
    d3d_desc.BorderColor[1] = desc.border_clr.g();
    d3d_desc.BorderColor[2] = desc.border_clr.b();
    d3d_desc.BorderColor[3] = desc.border_clr.a();
    d3d_desc.MinLOD = desc.min_lod;
    d3d_desc.MaxLOD = desc.max_lod;

    TIFHR(d3d_device->CreateSamplerState(&d3d_desc, sampler_state_.put()));
}

D3D11RenderStateObject::D3D11RenderStateObject(RasterizerStateDesc const & rs_desc, DepthStencilStateDesc const & dss_desc,BlendStateDesc const & bs_desc)
    : RenderStateObject(rs_desc, dss_desc, bs_desc)
{
    auto& re = checked_cast<D3D11RenderEngine&>(Context::Instance().RenderEngineInstance());
    ID3D11Device* d3d_device = re.D3DDevice();

    // 光栅化状态描述
    D3D11_RASTERIZER_DESC d3d_rs_desc;
    d3d_rs_desc.FillMode = D3D11Mapping::Mapping(rs_desc.polygon_mode);             // 填充模式
    d3d_rs_desc.CullMode = D3D11Mapping::Mapping(rs_desc.cull_mode);                // 裁剪模式
    d3d_rs_desc.FrontCounterClockwise = rs_desc.front_face_ccw;                     // 是否三角形顶点按逆时针排布时为正面
    d3d_rs_desc.DepthBias = static_cast<int>(rs_desc.polygon_offset_units);         // 深度偏移相关，目前忽略
    d3d_rs_desc.DepthBiasClamp = rs_desc.polygon_offset_units;                      // 深度偏移相关，目前忽略
    d3d_rs_desc.SlopeScaledDepthBias = rs_desc.polygon_offset_factor;               // 深度偏移相关，目前忽略
    d3d_rs_desc.DepthClipEnable = rs_desc.depth_clip_enable;                        // 是否允许深度测试将范围外的像素进行裁剪，默认TRUE
    d3d_rs_desc.ScissorEnable = rs_desc.scissor_enable;                             // 是否允许指定矩形范围的裁剪，若TRUE，则需要在RSSetScissor设置像素保留的矩形区域
    d3d_rs_desc.MultisampleEnable = rs_desc.multisample_enable;                     // 是否允许多重采样
    d3d_rs_desc.AntialiasedLineEnable = false;                                      // 是否允许反走样线，仅当多重采样为FALSE时才有效
    //d3d_rs_desc.ForcedSampleCount = 0; // TODO: Add support to forced sample count
    TIFHR(d3d_device->CreateRasterizerState(&d3d_rs_desc, rasterizer_state_.put()));

    D3D11_DEPTH_STENCIL_DESC d3d_dss_desc;
    d3d_dss_desc.DepthEnable = dss_desc.depth_enable;                                                       // 是否开启深度测试
    d3d_dss_desc.DepthWriteMask = D3D11Mapping::Mapping(dss_desc.depth_write_mask);                         // 深度值写入掩码
    d3d_dss_desc.DepthFunc = D3D11Mapping::Mapping(dss_desc.depth_func);                                    // 深度比较函数
    d3d_dss_desc.StencilEnable = dss_desc.front_stencil_enable;                                             // 是否开启模板测试
    d3d_dss_desc.StencilReadMask = static_cast<uint8_t>(dss_desc.front_stencil_read_mask);                  // 模板值读取掩码
    d3d_dss_desc.StencilWriteMask = static_cast<uint8_t>(dss_desc.front_stencil_write_mask);                // 模板值写入掩码
    // 对正面朝向的三角形进行深度/模板操作描述
    d3d_dss_desc.FrontFace.StencilFailOp = D3D11Mapping::Mapping(dss_desc.front_stencil_fail);                      
    d3d_dss_desc.FrontFace.StencilDepthFailOp = D3D11Mapping::Mapping(dss_desc.front_stencil_depth_fail);
    d3d_dss_desc.FrontFace.StencilPassOp = D3D11Mapping::Mapping(dss_desc.front_stencil_pass);
    d3d_dss_desc.FrontFace.StencilFunc = D3D11Mapping::Mapping(dss_desc.front_stencil_func);
    // 对背面朝向的三角形进行深度/模板操作的描述
    d3d_dss_desc.BackFace.StencilFailOp = D3D11Mapping::Mapping(dss_desc.back_stencil_fail);
    d3d_dss_desc.BackFace.StencilDepthFailOp = D3D11Mapping::Mapping(dss_desc.back_stencil_depth_fail);
    d3d_dss_desc.BackFace.StencilPassOp = D3D11Mapping::Mapping(dss_desc.back_stencil_pass);
    d3d_dss_desc.BackFace.StencilFunc = D3D11Mapping::Mapping(dss_desc.back_stencil_func);
    TIFHR(d3d_device->CreateDepthStencilState(&d3d_dss_desc, depth_stencil_state_.put()));

    // 初始化混合状态
    D3D11_BLEND_DESC d3d_bs_desc;
    d3d_bs_desc.AlphaToCoverageEnable = bs_desc.alpha_to_coverage_enable;
    d3d_bs_desc.IndependentBlendEnable = /*caps.independent_blend_support ?*/ bs_desc.independent_blend_enable /*: false*/;
    for (int i = 0; i < 8; ++ i)
    {
        uint32_t const rt_index = /*caps.independent_blend_support ? i :*/ 0;

        d3d_bs_desc.RenderTarget[i].BlendEnable = bs_desc.blend_enable[rt_index];
        //d3d_bs_desc.RenderTarget[i].LogicOpEnable = bs_desc.logic_op_enable[rt_index];
        d3d_bs_desc.RenderTarget[i].SrcBlend = D3D11Mapping::Mapping(bs_desc.src_blend[rt_index]);
        d3d_bs_desc.RenderTarget[i].DestBlend = D3D11Mapping::Mapping(bs_desc.dest_blend[rt_index]);
        d3d_bs_desc.RenderTarget[i].BlendOp = D3D11Mapping::Mapping(bs_desc.blend_op[rt_index]);
        d3d_bs_desc.RenderTarget[i].SrcBlendAlpha = D3D11Mapping::Mapping(bs_desc.src_blend_alpha[rt_index]);
        d3d_bs_desc.RenderTarget[i].DestBlendAlpha = D3D11Mapping::Mapping(bs_desc.dest_blend_alpha[rt_index]);
        d3d_bs_desc.RenderTarget[i].BlendOpAlpha = D3D11Mapping::Mapping(bs_desc.blend_op_alpha[rt_index]);
        // d3d_bs_desc.RenderTarget[i].LogicOp
        //     = caps.logic_op_support ? D3D11Mapping::Mapping(bs_desc.logic_op[rt_index]) : D3D11_LOGIC_OP_NOOP;
        d3d_bs_desc.RenderTarget[i].RenderTargetWriteMask
            = static_cast<UINT8>(D3D11Mapping::MappingColorMask(bs_desc.color_write_mask[rt_index]));
    }

    TIFHR(d3d_device->CreateBlendState(&d3d_bs_desc, blend_state_.put()));
}

void D3D11RenderStateObject::Active()
{
    auto& d3d11_re = checked_cast<D3D11RenderEngine&>(Context::Instance().RenderEngineInstance());
    d3d11_re.RSSetState(rasterizer_state_.get());
    d3d11_re.OMSetDepthStencilState(depth_stencil_state_.get(), dss_desc_.front_stencil_ref);
    d3d11_re.OMSetBlendState(blend_state_.get(), bs_desc_.blend_factor, bs_desc_.sample_mask);
}



}