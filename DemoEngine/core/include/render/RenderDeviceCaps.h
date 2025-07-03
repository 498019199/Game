#pragma once

#include <render/Renderable.h>

namespace RenderWorker
{

struct RenderDeviceCaps
{
    bool gs_support : 1;
    bool cs_support : 1;
    bool hs_support : 1;
    bool ds_support : 1;
};


}