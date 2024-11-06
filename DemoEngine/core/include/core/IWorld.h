#pragma once

class IRenderable;

namespace CoreWorker
{
class IWorld 
{
public:
    virtual IWorld() = default;
    virtual ~IWorld() = default;

    void AddRenderable(const IRenderable* obj);
    
    void Update();
};
}