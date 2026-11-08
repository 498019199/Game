#pragma once
#include <render/RenderMaterial.h>
#include <render/RenderView.h>
#include <memory>
namespace EditorWorker
{
// Isolated offscreen geometry; never registers nodes in the user's scene.
class EditorMaterialPreview
{
public:
    EditorMaterialPreview();
    ~EditorMaterialPreview();
    void Render(RenderWorker::RenderMaterialPtr const& material, int shape, float yaw, float distance);
    RenderWorker::ShaderResourceViewPtr const& Image() const;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
}
