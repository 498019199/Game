#pragma once
#include <base/ZEngine.h>
#include <render/Renderable.h>

namespace RenderWorker
{

class ZENGINE_CORE_API RenderableTriangle : public Renderable
{

};

class ZENGINE_CORE_API RenderableBox : public Renderable
{
public:
    RenderableBox(float width, float height, float depth, const Color & color);
};

class ZENGINE_CORE_API RenderableSphere : public Renderable
{
public:
    RenderableSphere(float radius, int levels, int slices, const Color & color);
};

class ZENGINE_CORE_API RenderablePlane : public Renderable
{
public:
    RenderablePlane(float length, float width,
		int length_segs, int width_segs, bool has_tex_coord, bool has_tangent);
};

class ZENGINE_CORE_API TerrainRenderable : public RenderablePlane
{
public:
    explicit TerrainRenderable(TexturePtr const & height_map, TexturePtr const & normal_map);
    void OnRenderBegin() override;
};


class ZENGINE_CORE_API RenderDecal : public Renderable
{
public:
    
};

}