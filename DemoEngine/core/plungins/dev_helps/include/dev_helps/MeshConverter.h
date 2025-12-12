#pragma once

#include <render/Mesh.h>

#include <string_view>

#include <dev_helps/MeshMetadata.h>

struct aiNode;
struct aiScene;

namespace RenderWorker
{
class ZENGINE_CORE_API MeshConverter final
{
public:
    RenderModelPtr Load(MeshMetadata const & metadata);
    void Save(RenderModel& model, std::string_view output_name);

    static bool IsSupported(std::string_view input_name);
};
}
