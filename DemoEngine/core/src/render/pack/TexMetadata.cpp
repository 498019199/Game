
#include "TexMetadata.h"

namespace CoreWorker
{
void TexMetadata::Load(std::string_view name)
{
    Load(name, true);
}

void TexMetadata::Load(std::string_view name, bool assign_default_values)
{
    TexMetadata new_metadata;
}

void TexMetadata::Save(std::string const & name) const
{
    
}
}