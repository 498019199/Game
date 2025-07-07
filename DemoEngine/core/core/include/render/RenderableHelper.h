#pragma once

#include <Render/Renderable.h>

namespace RenderWorker
{

class RenderableTriangle : public Renderable
{

};

class RenderableBox : public Renderable
{
public:
    RenderableBox(float width, float height, float depth, const Color & color);
};

class RenderableSphere : public Renderable
{
public:
    RenderableSphere(float radius, int levels, int slices, const Color & color);
};

class RenderablePlane : public Renderable
{
public:
    RenderablePlane(float width, float depth, float texU, float texV, const Color & color);
};




class RenderDecal : public Renderable
{
public:
    
};

}