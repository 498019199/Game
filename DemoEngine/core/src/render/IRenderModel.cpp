#include <render/IRenderModel.h>
#include "pack/MeshLoader.h"

namespace CoreWorker
{
void IRenderModel::NumMaterial(int size)
{
    materials_.resize(size);
}

RenderMaterialPtr& IRenderModel::GetMaterial(int index)
{
    return materials_[index];
}

const RenderMaterialPtr& IRenderModel::GetMaterial(int index) const 
{
    return materials_[index];
}

bool IRenderModel::LoadModel(ModelFileType type, const char* filename)
{
    if(ModelFileType::MODEL_FILE_FBX == type || ModelFileType::MODEL_FILE_OBJECT == type)
    {
        MeshLoader Loader(this);
        MeshMetadata meshdata;
        meshdata.lod_file_names_.emplace_back(filename);
        Loader.LoadFromAssimp(filename, meshdata);
    }
    else 
    {

    }

    return true;
}


}