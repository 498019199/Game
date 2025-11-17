#pragma once
#include <base/ZEngine.h>
#include <render/Texture.h>
#include <render/RenderMaterial.h>
#include <audio/Audio.h>
#include <render/Mesh.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx11.h>

using namespace CommonWorker;
using namespace RenderWorker;

namespace EditorWorker
{
struct EditorSetting
{
    int hierarchyWidth {0};
    int hierarchyHeight {0};
    int consoleWidth {0};
    int consoleHeight {0};
    int projectWidth {0};
    int projectHeight {0};
    int inspectorWidth {0};
    int inspectorHeight {0};
    int mainBarWidth {0};
    int mainBarHeight {0};

    int srcWidth {0};
    int srcHeight {0};

    int gameViewWidth {0};
    int gameViewHeight {0};

    bool is_game_started_ {false};
    bool is_game_paused_ {false};
};


enum class AssetType
{
    Other,
    Folder,
    Material,
    DeferredMaterial,
    RayTracingMaterial,
    Prefab,
    Script,
    Shader,
    Texture,
    Scene,
    Model,
    RayTracingShader,
    Audio,
    Text,
    Count,
};

struct AssertBaseInfo
{
    std::string name;

    virtual ~AssertBaseInfo() = default;
};
using AssertBaseInfoPtr = std::shared_ptr<AssertBaseInfo>;


struct AssetScriptInfo: public AssertBaseInfo
{
    std::string preview;
};
using AssetScriptInfoPtr =  std::shared_ptr<AssetScriptInfo>;


struct AssetShaderInfo: public AssertBaseInfo
{
    std::string preview;
};
using AssetShaderInfoPtr =  std::shared_ptr<AssetShaderInfo>;


struct AssetTextureInfo: public AssertBaseInfo
{
    std::string format;
    TexturePtr  texture;
};
using AssetTextureInfoPtr =  std::shared_ptr<AssetTextureInfo>;


struct AssetMaterialInfo: public AssertBaseInfo
{
    RenderMaterialPtr material;
};
using AssetMaterialInfoPtr =  std::shared_ptr<AssetMaterialInfo>;


struct AssetModelInfo: public AssertBaseInfo
{
    std::string     format;
    RenderModelPtr  model;
};
using AssetModelInfoPtr =  std::shared_ptr<AssetModelInfo>;


struct AssetAudioInfo: public AssertBaseInfo
{
    AudioBufferPtr audio_buff_;
};

class EditorPanel
{
public:
    EditorPanel() = default;
    ~EditorPanel() = default;

    virtual void OnRender(const EditorSetting& setting) = 0;
    virtual void OnResize() = 0;
private:
};
using EditorPanelPtr = std::shared_ptr<EditorPanel>;
}