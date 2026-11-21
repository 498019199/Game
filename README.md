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


# 使用方式
在 zengine/bin/win_x64/ 目录下运行：
```shell
.\ZENGINE_model_convert.exe models/Spring.obj
.\ZENGINE_model_convert.exe models/Foo.glb models/Bar.fbx
.\ZENGINE_model_convert.exe -f models/Spring.obj          # 强制重建
.\ZENGINE_model_convert.exe --assets-dir D:\my\models MyModel.obj
```



工程里用的是 Tracy v0.13.1，官方 GUI 下载方式：
打开 Releases：https://github.com/wolfpld/tracy/releases/tag/v0.13.1
下载 windows-0.13.1.zip
解压后运行 tracy-profiler.exe
用法：先开 Tracy GUI → 再跑 Editor（需 ZENGINE_ENABLE_TRACY=ON）。On-demand 模式下连上后才会开始采数据。
文档（可选）：同一页的 tracy.pdf。

Windows 默认加载 `engine/plugins/render/d3d12`，也可以选择 `d3d11`。macOS 加载 `engine/plugins/render/metal`。三个目录各自定义渲染插件、工厂和设备实现。
    
gm
/createnpc 100000 1
/createnpc 300001 1

## CMake 目录组织

构建辅助脚本按模块管理，各目录的 `CMakeLists.txt` 作为构建入口。

- `engine/cmake/config/`：编译器选项和平台识别。
- `engine/cmake/dependencies/`：第三方源码获取、版本切换和补丁工具；库自身的构建入口仍在 `engine/3rd/`。
- `gameplay/cmake/config/`：.NET SDK、宿主包和输出目录。
- `gameplay/cmake/dependencies/`：Protobuf/Abseil 获取与第三方警告隔离。
- `gameplay/cmake/targets/`：协议生成、托管发布、原生宿主/客户端和测试目标，按依赖顺序加载。
- `gameplay/cmake/EngineIntegration.cmake`：原生 game 接入 C# 的入口。

`include()` 保留调用模块的源码/构建目录作用域；入口使用绝对脚本路径，各 target 的名称、输出位置和已有构建命令保持不变。

## CMake 依赖约定

- 引擎第三方源码、固定版本和构建配置集中维护在 `engine/3rd`；可独立构建的 gameplay 通过自身 `cmake/dependencies` 管理 Protobuf/Abseil。各模块链接已有 target，不重复下载同一个库。
- 模块通过 `target_link_libraries` 声明直接依赖；公共头文件路径由所属 target 的 `PUBLIC` include 传递，不手写其他模块的源码路径。
- 公共头文件需要的依赖使用 `PUBLIC`；仅实现使用的依赖和可执行程序的依赖使用 `PRIVATE`。预编译头由各 target 自行选择，不向下游传播。
- RmlUi 的静态运行库由 `ZENGINE_core` 持有。只使用类型和枚举的模块依赖 `ZENGINE_RmlUiHeaders`，实际 UI 操作通过 `UIManager` 调用，避免多个动态库各自包含 RmlUi 运行时。
- 编辑器通过 `ZENGINE_Render_D3D12` 或 `ZENGINE_Render_Metal` 获取原生绘图对象。游戏逻辑使用引擎接口，桌面窗口和键鼠事件由 Win32 和 Cocoa 提供，经引擎的 Window 接口传递。

## 引擎源码目录

- `engine/common`：通用工具和数学库。
- `engine/runtime`：应用、资源、场景及渲染接口；TableGen 生成任务也在此定义。
- `engine/plugins`：渲染、输入、音频和资源转换插件。
- `engine/3rd`、`engine/cmake`、`engine/tests`：第三方依赖、构建配置和引擎测试。

目录统一使用小写（第三方依赖保留上游命名），文件名、C++ 类型和 `ZENGINE_core` 等 CMake target 名称保持不变。资源和运行输出分别位于 `zengine/assets`、`zengine/bin`。迁移已有构建目录时需重新配置 CMake；旧平台缓存可以保留作备份，再建立新的构建目录。

## VS Code 编译与调试（Windows x64）

在 VS Code 中打开仓库根目录。需要 Microsoft C/C++ 扩展、Visual Studio 2022 的 MSVC x64 工具和 Windows SDK，以及 PATH 中的 CMake 3.28 或更新版本和 Git。CMake Tools 扩展可为 IntelliSense 提供目标的头文件路径和编译选项。首次配置会下载缺失的 C++ 第三方依赖。

- **编译 Editor**按 `Ctrl+Shift+B`。要编译 Game，在命令面板执行 `Tasks: Run Task`，选择 `CMake: Build Game (Debug)`。
- **断点调试**在 `editor/main.cpp` 或 `editor/game_main.cpp` 的可执行代码行设置断点。在「运行和调试」选择 `vscode-editor` 或 `vscode-game`，按 `F5`。VS Code 会先编译相应的 Debug 目标，再从 `zengine/bin/win_x64` 启动。

默认构建使用 C++，NPC 配置来自 `zengine/assets/config/npc.json`，GM 控制台支持 `help`、`clear` 和 `/createnpc <id>`。需要 C# 战斗、技能及 `/gas` 时，可用 `-DZENGINE_ENABLE_MANAGED_GAMEPLAY=ON` 配置 CMake，此模式需要 .NET 10 SDK。

## macOS Editor 与 RmlUi 验证

macOS 的 Metal 渲染后端使用固定版本的 glslang 将引擎 HLSL 编译为 SPIR-V，再通过 SPIRV-Cross 生成 Metal Shading Language。常量缓冲布局、资源槽位和顶点语义由反射映射；编译器源码由 `engine/3rd/shader_tools` 管理，无需另外安装命令行 Shader 编译器。当前接入顶点和片元阶段。

```sh
cmake --preset macos-apple-clang
cmake --build --preset macos-apple-clang-debug --target ZENGINE_editor ZENGINE_rmlui_gpu_tests
cd zengine/bin/darwin_arm64
./ZENGINE_rmlui_gpu_tests
./ZENGINE_editor --backend=Metal
```

`ZENGINE_rmlui_gpu_tests` 使用真实 Metal GPU 绘制测试文档并读取离屏像素，验证位置/颜色、裁剪、字体纹理及隐藏/显示；不加载或修改用户场景。macOS 使用系统 Arial 字体映射已有 RmlUi 文档的字体族，字体文件不复制进仓库。

## 音频后端与回归测试

`zengine/assets/KlayGE.cfg` 的 `audio_factory="auto"` 按平台选择音频后端：Windows 使用 XAudio，macOS 使用 Core Audio。旧配置中不可用的插件名会回退到当前平台的默认后端。构建编辑器和游戏程序时会一并构建所需插件。

Core Audio 支持 PCM 音效预加载与多声部、音乐流式播放、循环、停止和音量控制，现用于编辑器音频预览；位置、速度和朝向接口保留状态，尚未实现 3D 空间音效。

```sh
cmake --build --preset macos-apple-clang-debug --target ZENGINE_editor ZENGINE_editor_tests
cd zengine/bin/darwin_arm64
./ZENGINE_editor_tests_clang210
```

音频回归测试使用静音 PCM 实际打开播放设备，检查播放完成、循环和停止时解码器同步。播放测试需要系统提供可用的音频输出设备。

## 粒子特效

要直接查看三种特效，选择 **File > Open Scene** 并输入 `scenes/ParticleShowcase.scene`，从左到右是火焰、烟雾和火花。打开含粒子的场景时，编辑器使用场景已保存的相机位置和视角开始预览；切到 **Scene** 可选择特效并使用变换手柄。示例不依赖角色模型，默认场景保持原样。

编辑器顶部选择 **Create > Effects > Fire / Smoke / Sparks / Explosion / Heat haze**，或在 Hierarchy 空白处右键选择 **Effects**。特效创建在视野前方并自动聚焦；选中 Hierarchy 中的特效后，在 Inspector 的 Emission、Motion、Appearance、Light and refraction 分组中调参。支持 Play、Pause、Stop、Restart，以及在 Scene 视图中使用 T/R/S 移动、旋转、缩放，F 聚焦。

`scenes/BlastShowcase.scene` 展示循环爆炸、发光闪烁和拉伸火花。Explosion 预设自身为一次性爆发；示例为便于观察将其设为每两秒循环。`Burst count` 在开始时发射指定数量的粒子，循环时每个 `Cycle duration` 再次发射；`Rate / second` 可同时持续发射。运行时可按 **Emit burst now** 追加一次爆发。达到数量上限时跳过多余粒子，不会积压后补发。

火焰和火花使用加法混合，烟雾使用 Alpha 混合，具有模型遮挡关系。`Simulation space` 选择 Local 时粒子跟随发射器移动；选择 World 时位置、速度、加速度和大小在出生时转换到世界空间，随后移动发射器不会拖动已发出的粒子。适合移动火花、尾气和爆炸。旧场景缺省使用 Local，保持原行为。

`Rotation min/max` 和 `Spin min/max` 控制粒子的初始旋转与旋转速度。`Velocity stretch` 沿屏幕中投影的运动方向拉长粒子，形成火花或弹道拖尾；它的单位是秒，0 表示关闭。

`Emissive intensity` 控制特效自身的亮度，`Glow strength` 增加柔和光晕（不是全屏 Bloom）。`Flicker frequency` 和 `Flicker amount` 控制闪烁频率与深度。选择 **Heat haze** 预设或 Distortion 混合模式，使用 `Distortion` 像素强度扭曲后方场景；将它放在模型、地面纹理等有细节的背景前，纯色背景看不出明显折射。扭曲会读取当前场景颜色，每个扭曲发射器需要一次颜色复制。

原生渲染后端的热浪扭曲支持编辑器 **Scene** 和 **Game** 的离屏视图；直接绘制到交换链的运行路径不支持读取当前颜色，因此暂不支持热浪扭曲。

此版本使用 CPU 粒子模拟。参数变化会重置预览，播放/暂停属于临时预览状态；暂停时不会追加爆发。

**File > Save Scene** 将特效参数、名称和变换保存为 `.scene` 的 `ParticleEffects` 数组，再次打开场景时自动播放。旧场景不需要修改；移除特效后保存会同步删除对应记录。独立游戏程序加载同一场景也会显示普通 Alpha / Additive 粒子；热浪扭曲的运行目标限制见上文。

粒子回归测试：

```sh
cmake --build --preset macos-apple-clang-debug --target ZENGINE_editor_tests ZENGINE_particle_gpu_tests
cd zengine/bin/darwin_arm64
./ZENGINE_editor_tests_clang210 --gtest_filter='ParticleSystem.*:SceneParticles.*'
./ZENGINE_particle_gpu_tests_clang210 Metal --write-previews
```

GPU 测试检查火焰、烟雾、火花和爆炸的实际像素，覆盖深度遮挡、透明粒子不写深度、缓冲扩容、世界空间、速度拖尾、发光、闪烁、背景折射，以及画面缓存的尺寸变化与复用；预览图写入系统临时目录。烟雾在单个发射器内按深度排序，多个交叠发射器尚未进行全局逐粒子排序。
