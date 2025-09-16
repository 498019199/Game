# Framework
core 渲染核心
math 数学
common 通用，常用或者专用数据结构
3rd 第三方库
externals 目录下面cmake下载lib
[ini库](https://github.com/metayeti/mINI)
包管理vcpkg

ID3D11Device接口用于检测显示适配器功能和分配资源
ID3D11DeviceContext接口用于设置管线状态、将资源绑定到图形管线和生成渲染命令。
IDXGIFactory 是创建其他接口的工厂函数，可以假想为一台主机中所有与图形相关的硬件集合

# 图形渲染技术：
DX9： 支持固定功能管线和有限的可编程管线
DX11：增加了**细分曲面、计算着色器等技术**，提高了多线程处理能力。
DX12：进一步优化了**多线程效率**，支持异步计算和**底层硬件的更直接控制**。

开发日志
2025年7月7日
从studyrecord\Introduction to DirectX 11 3D Game Engine Programming 移植相关代码，抽象api

# 新增特性
Direct3D 11.2
低延迟渲染：新增 IDXGISwapChain2 接口，支持 DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL 翻转模式，减少帧延迟（现代应用广泛使用）。
HDR 显示支持：新增对高动态范围显示的色彩空间支持（如 DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020）。

Direct3D 11.4


2025年9月16日
移植klayGE 图片读取相关代码，完全没有看，仅仅移植