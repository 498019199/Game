# Game
基于C++最新标准。Math 主要《3D数学基础：图形与游戏开发》，看KlayGE完成模板化  。
Container 来自于《数据结构与算法分析 : C语言描述》，参看部分STL内容  。
解析obj,mtl。用库tiny_obj_loader 。
图片解析移植 COCOS 。
文件系统移植klayGE。

流水
《3D游戏编程大师技巧》上下册

2018年9月5日
模型obj,mtl修改为tiny_obj_loader加载完成

2018年9月12日
完成模型轮廓线显示

2018年9月16日
把main中的创建窗口，窗口流程控制拆分成2给类。
Windows
创建窗口：消息回调，以后方便跨平台
App：窗口流程

2018年9月23日
移植klayGE中的Input,Minput模块。运行成功

2018年9月24日
移植CameraContral,FirstPersonCameraController

2018年9月27日
完成轮廓线显示，输入系统调试Ok

2018年9月29日
添加玩家对象
