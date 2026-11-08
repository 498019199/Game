# C# 游戏逻辑与 Protocol Buffers

现有 C++ GAS 已迁移到 `shared/gas/`，不再保留第二套 C++ 战斗规则。编辑器和 GameApp 默认创建 C# 游戏会话，通过同一份 protobuf 协议调用；服务器直接运行同一个 C# 核心。

`game/` 仍有用途：模型、材质、场景、摄像机、RmlUi 和资源加载属于原生引擎适配。它只保留这些表现代码及 C# 宿主入口；技能、属性、效果、标签、战斗推进、GM 命令解析、NPC 定义均在 C# 中。不能把原生渲染对象搬进服务器或简单删除整个目录。

## 模块

- `proto/gameplay.proto`：唯一通信契约，proto3，包名 `zengine.gameplay.v1`。
- `protocol/`：Google.Protobuf 3.36.2；C# 生成物随源码提交，CMake 校验其与 schema 一致。
- `shared/gas/`：netstandard2.1/C# 8，游戏世界、属性、效果、层级标签、时间线、冷却、事件和 GM 规则。
- `runtime/`：protobuf 会话分发、配置编译、网络分帧；.NET 10。
- `bridge/`：.NET 原生入口，传递会话句柄和 protobuf 字节；不再传固定战斗结构体。
- `client/`：C++ .NET 宿主、RAII 游戏会话、请求回放客户端。C++ 没有伤害/技能公式。
- `server/`：无渲染 .NET 10 服务器，支持回放、烟雾测试和 TCP；每个连接为一个独立开发测试房间。
- `tests/`：带独立预期的规则测试，以及 C++ 内嵌/C# 独立/TCP/容器一致性测试。

## CMake 组织

`CMakeLists.txt` 保留工程声明、链接策略和模块加载顺序；`cmake/config/Dotnet.cmake` 负责运行环境，`cmake/dependencies/Protobuf.cmake` 负责依赖，`cmake/targets/` 分别定义 Protocol、Managed、Native、Tests。`cmake/EngineIntegration.cmake` 是 game 的接入入口。脚本通过 `include()` 加载，源码与输出路径仍以 gameplay 的目录为基准。

## 构建与运行

需要 .NET 10 SDK、CMake 3.24+、C++17 编译器、测试用 Python 3，以及首次恢复依赖的网络访问。Protobuf 36.2 与 Abseil 20250512.1 使用固定 URL 和 SHA256。本机 SDK 位于 Git 忽略的 `.dotnet/`，CMake 自动发现；其他机器需安装 SDK。Windows 宿主代码已提供，实际验证平台见文末。

```sh
cd /Users/zhangbei/Code/Game/gameplay
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 6
ctest --test-dir build --output-on-failure
./build/bin/gameplay_client
./.dotnet/dotnet ./build/server/Game.Server.dll --smoke
./.dotnet/dotnet ./build/server/Game.Server.dll --listen 127.0.0.1 7777
```

修改 `.proto` 后先执行 `cmake --build build --target gameplay_codegen_refresh`，再正常构建。CMake 每次运行增量 MSBuild，避免 C# 修改没有更新；已有进程需要重启。

## 调用协议

C++ 中持有一个 `Gameplay::GameSession`，构造 protobuf 请求：

```cpp
Gameplay::Protocol::Request request;
auto* cast = request.mutable_cast_skill();
cast->set_source_id(1);
cast->set_target_id(2);
cast->set_skill_id(1001);
auto response = session.Call(std::move(request));
// 检查 response.status()，消费 entities 快照和 events。
```

C# 服务端直接持有 `GameWorld`（或使用封装好的 `ProtocolSession`）：

```csharp
var response = world.Handle(new Request {
    Version = 1, RequestId = 42,
    CastSkill = new CastSkill { SourceId = 1, TargetId = 2, SkillId = 1001 }
});
```

先用 CreateEntity 建立角色，再发送 CastSkill/Tick 等命令。时间单位是毫秒；属性和倍率沿用旧 GAS 的 float/归一化比例（0.2 表示 20%），不是上一版演示的整数千分比。BasicStrike 使用现有配置：200ms 命中，300ms 结束；ManaBolt 消耗 10 MP，250ms 命中，400ms 结束，冷却 1000ms。

- 进程内：C ABI 的 Create/Exchange/Release/Destroy 传递 protobuf 缓冲区；响应由 .NET 分配，C++ RAII 调用 Release，避免跨分配器释放。
- TCP/回放：4 字节大端无符号长度 + protobuf 消息，单帧上限 4 MiB。不依赖 JSON、结构体内存布局或主机字节序。
- version=1，request_id 回显。未知字段可兼容，未知版本、非法输入和 malformed protobuf 返回明确错误。
- 会话按房间隔离；单个 C++ GameSession 由一个调用线程使用，托管分发同一会话加锁。销毁会话会释放房间状态，不卸载进程内 CLR。
- TCP 是用于开发验证的管理协议，允许创建实体/设置属性/GM 命令，尚未接入登录、玩家权限和正式房间调度，不是公网玩家协议。

## 编辑器与原生 game 适配

调用链：`EditorManager/GameApp::OnCreate` → `GameContext::Startup` → C# 会话。旧 `ZENGINE_ENABLE_CSHARP_DEMO` 开关已移除，C# 是唯一玩法实现，没有 C++ GAS 回退路径。

`AScene::UpdateDetailedMeshes` 的首个渲染 pass 推进一次 C# 时间，保留不足 1ms 的余量。GM 窗口将文本作为 ExecuteGm protobuf 命令交给 C#；C++ 只显示结果或根据 NPC 描述实例化模型。NPC ID/Prefab 定义来自 C# 目录，原生缓存只是与已加载渲染资产的映射。

`zengine/assets/config/{skills,skill_flows,npc}.json` 是唯一配置源。构建时 C# 编译为 `game.catalog.pb`，客户端和服务器使用同一份二进制。新配置验证失败则构建失败，不部分加载。旧 `game/src/game/gas` 和 `game/include/game/gas` 已删除。

编辑器按原来的 CMake preset 构建即可，不需要额外开关。部署原生客户端需携带托管发布目录、nethost 动态库以及匹配架构的 .NET Runtime；开发路径可用 DOTNET_ROOT 和 GAMEPLAY_MANAGED_DIR 覆盖。

## Docker / Colima

构建上下文是仓库根目录。仓库根目录和 Dockerfile 专用 ignore 仅放行 gameplay 源码和公开配置，不包含 engine、私有资产、本机 SDK 或编译缓存；本次实际发送的构建上下文为 364 KB。

```sh
cd /Users/zhangbei/Code/Game
docker build -f gameplay/Dockerfile -t zengine-game-server:protobuf-dev .
docker run --rm zengine-game-server:protobuf-dev --smoke
docker run --rm --name zengine-game-server-test -p 127.0.0.1:17777:7777 zengine-game-server:protobuf-dev
```

镜像中只有 .NET Runtime、服务器及托管依赖，以非 root 用户运行，默认监听容器内 7777。可以在宿主运行同一套验证，增加 `--endpoint 127.0.0.1:17777` 检查容器；复测命令如下：

```sh
cd /Users/zhangbei/Code/Game/gameplay
DOTNET_ROOT="$PWD/.dotnet" python3 tests/verify.py \
  --client "$PWD/build/bin/gameplay_client" --managed "$PWD/build/managed" \
  --dotnet "$PWD/.dotnet/dotnet" --server "$PWD/build/server/Game.Server.dll" \
  --tests "$PWD/build/tests/Game.Tests.dll" --work "$PWD/build/verification" \
  --endpoint 127.0.0.1:17777
```

修改脚本/配置后构建新镜像并重启，不实现带状态的在线热重载。

## 迁移行为

保留属性合并 `(base + sumAdd) * (1 + sumPercent)`、HP 钳制、归一化倍率、RulePlan 扣蓝、技能时间线、周期/即时/无限效果、GM smoke/skillsmoke。清理了旧实现中的边界问题：重叠效果标签不会被提前移除，冷却实际阻止重复施法，周期 tick 不越过到期时间，目标删除取消施法，多段时间线保留跨段时间余量。

网络快照不会自动创建场景中的可视角色；网络预测、正式角色移动/AI/任务系统和鉴权不在已有 game 功能中，本次不虚构这些系统。原生场景/材质功能继续由引擎负责。

## 验证范围（2026-11-06）

- macOS arm64：C++ 宿主与 C# 独立服务器完成 58 条 Protobuf 请求回放，状态快照及事件逐字节相同；核心程序集和配置二进制一致。
- Colima Linux arm64：容器战斗/技能 smoke、双连接各 58 条请求、拆分帧、非法报文、超大帧拒绝及重连通过；非 root UID 1654，SIGTERM 正常退出（exit 0）。
- 覆盖属性、效果生命周期、标签、技能时间线、扣蓝/冷却、死亡、删除/复用实体、配置校验、会话隔离和创建/销毁。
- game 的 7 个原生编译单元以及 EditorManager/GameApp 的语法编译检查通过；检查时将现有引擎警告降级，未修改 engine/editor。编辑器 GUI 完整启动和 Windows/Unity/UE 集成未在本次验证中运行。
