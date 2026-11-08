#pragma once

#include <render/Renderable.h>

namespace RenderWorker 
{

class ZENGINE_CORE_API RenderableSkyBox : public Renderable
{
public:
    RenderableSkyBox();

    virtual void Technique(RenderEffectPtr const & effect, RenderTechnique* tech);
    void CubeMap(TexturePtr const & cube);
    void CompressedCubeMap(TexturePtr const & y_cube, TexturePtr const & c_cube);

    void OnRenderBegin();

    // For deferred only

    virtual void Pass(PassType type);

protected:
    RenderEffectParameter* depth_far_ep_;
    RenderEffectParameter* inv_mvp_ep_;
    RenderEffectParameter* skybox_cube_tex_ep_;
    RenderEffectParameter* skybox_Ccube_tex_ep_;
    RenderEffectParameter* skybox_compressed_ep_;
};

}