#pragma once
#include <render/IRenderMaterial.h>
#include <vector>

namespace CoreWorker
{
enum class VertexElementType: uint8_t 
{
    // vertex positions
    VEU_Position = 0,
    // vertex normals included (for lighting)
    VEU_Normal,
    // Vertex colors - diffuse
    VEU_Diffuse,
    // Vertex colors - specular
    VEU_Specular,
    // Vertex blend weights
    VEU_BlendWeight,
    // Vertex blend indices
    VEU_BlendIndex,
    // at least one set of texture coords (exact number specified in class)
    VEU_TextureCoord,
    // Vertex tangent
    VEU_Tangent,
    // Vertex binormal
    VEU_Binormal
};

enum class ModelFileType: uint8_t
{
    MODEL_FILE_OBJECT = 0,
    MODEL_FILE_FBX = 1,
};

struct VertexElement
{
    VertexElementType ElementType;
    uint8_t VertexIndex;
};

class IRenderModel
{
public:
    IRenderModel() = default;
    ~IRenderModel() = default;

    bool LoadModel(ModelFileType type, const char* modelfile);

    void NumMaterial(int size); 
    RenderMaterialPtr& GetMaterial(int index); 
    const RenderMaterialPtr& GetMaterial(int index) const; 
private:
    std::vector<RenderMaterialPtr> materials_;
};


}