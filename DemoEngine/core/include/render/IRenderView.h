#pragma once
#include <core/define.h>

namespace CoreWorker
{
class ShaderResourceView
{

};
using ShaderResourceViewPtr = std::shared_ptr<ShaderResourceView>;

class IRenderView
{
public:
    IRenderView() = default;
    ~IRenderView() = default;
};
}