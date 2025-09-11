#pragma once
#include <editor/EditorPanel.h>

namespace EditorWorker
{

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

class EditorProjectPanel: public EditorPanel
{
public:
    EditorProjectPanel();
    ~EditorProjectPanel();

    virtual void OnRender(const EditorSetting& setting) override;
    virtual void OnResize() override;

private:

};

}