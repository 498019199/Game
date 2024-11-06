#pragma once
#include <core/define.h>
#include <render/IRenderModel.h>

namespace CoreWorker
{
class TexMetadata
{
public:
    void Load(std::string_view name);
    void Load(std::string_view name, bool assign_default_values);
    void Save(std::string const & name) const;
};
}