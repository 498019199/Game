#include <render/Renderable.h>

#include <base/ZEngine.h>
#include <render/RenderFactory.h>

namespace RenderWorker
{


Renderable::Renderable()
    :rls_(1), active_lod_(-1)
{
}

Renderable::Renderable(std::wstring_view name)
    : name_(name), rls_(1)
{
}

Renderable::~Renderable()
{
}

RenderLayout& Renderable::GetRenderLayout() const
{
	return GetRenderLayout(active_lod_);
}

RenderLayout& Renderable::GetRenderLayout(uint32_t lod) const
{
    return *rls_[lod];
}

RenderEffect* Renderable::GetRenderEffect() const
{       
    return effect_.get();
}

RenderTechnique* Renderable::GetRenderTechnique() const
{
    return technique_;
}

void Renderable::NumLods(uint32_t lods)
{
    rls_.resize(lods);
}

uint32_t Renderable::NumLods() const
{
    return static_cast<uint32_t>(rls_.size());
}

void Renderable::ActiveLod(int32_t lod)
{
    if (lod >= 0)
    {
        active_lod_ = std::min(lod, static_cast<int32_t>(NumLods() - 1));
    }
    else
    {
        // -1 means automatic choose lod
        active_lod_ = -1;
    }
}

int32_t Renderable::ActiveLod() const
{
    return active_lod_;
}

void Renderable::Render()
{
    int32_t lod;
    if (active_lod_ < 0)
    {
        lod = 0;
    }
    else
    {
        lod = active_lod_;
    }

    const auto& effect = *GetRenderEffect();
    const auto& layout = GetRenderLayout(lod);
    const auto& tech = *GetRenderTechnique();
    auto& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
    re.Render(effect, tech, layout);
}

void Renderable::Material(const RenderMaterialPtr& mtl)
{
    
}
}