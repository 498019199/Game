#pragma once

#include <render/Renderable.h>

namespace RenderWorker
{

struct RenderDeviceCaps
{
    uint32_t max_texture_width;         // 2D 纹理在 U 轴（宽度）上允许的最大尺寸
    uint32_t max_texture_height;        // 2D 纹理在 V 轴（高度）上允许的最大尺寸
    uint32_t max_texture_depth;         // 2D 纹理在 W 轴（深度度）上允许的最大尺寸
    uint32_t max_texture_cube_size;     // 立方体贴图最大尺寸
    uint32_t max_texture_array_length;  // 2D 纹理数组（Texture2D Array）在数组维度上允许的最大元素数量
    uint8_t max_vertex_texture_units;   // 每个着色器阶段（如顶点着色器、像素着色器等）可绑定的采样器（Sampler）的最大数量
    uint8_t max_pixel_texture_units;
    uint8_t max_geometry_texture_units;
    uint8_t max_simultaneous_rts;       // 同时可绑定的渲染目标（Render Target Render Target）的最大数量
    uint8_t max_simultaneous_uavs;      // 像素着色器（PS）和计算着色器（CS）可同时绑定的无序访问视图（UAV，Unordered Access View）的最大数量
    uint8_t max_vertex_streams;         // 一个顶点声明（Vertex Declaration）中允许的最大顶点元素（Vertex Element）数量
    uint8_t max_texture_anisotropy;     // 各向异性过滤（Anisotropic Filtering）的最大支持级别

    bool gs_support : 1;
    bool cs_support : 1;
    bool hs_support : 1;
    bool ds_support : 1;
};


}