#pragma once

#include <common/XMLDom.h>

#include <string>
#include <string_view>

namespace RenderWorker
{
// Parse Unity-like ShaderLab Lite source into an effect XML tree
// compatible with RenderEffect::Load(XMLNode).
// Returns false and fills error_msg on failure.
bool ParseShaderLabLite(std::string_view source, CommonWorker::XMLNode& out_effect, std::string* error_msg = nullptr);
}
