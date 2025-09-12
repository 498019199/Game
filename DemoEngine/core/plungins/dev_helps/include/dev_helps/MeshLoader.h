#pragma once

#include <base/ZEngine.h>

#include <assimp/config.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
namespace RenderWorker
{
struct MeshMetadata
{
    float3 translation_ = float3::Zero();
	quater rotation_ = quater::Identity();
	float3 scale_ = float3(1, 1, 1);

    std::vector<std::string> lod_file_names_;
};

class MeshLoader
{
public:

    MeshLoader(IRenderModel* model);
    
    void LoadFromAssimp(std::string_view input_name, MeshMetadata const & metadata);

private:

    void BuildMeshData(std::vector<aiScene const*> const & scene_lods);

    void BuildMaterials(aiScene const * scene);

private:
    IRenderModel* render_model_;

    static uint32_t constexpr MAX_NUMBER_OF_TEXTURECOORDS = 8;
    struct Mesh
    {
        int mtl_id;
        std::string name;

        struct Lod
        {
            std::vector<float3> positions;
            std::vector<float3> tangents;
            std::vector<float3> binormals;
            std::vector<float3> normals;
            std::vector<Color> diffuses;
            std::vector<Color> speculars;
            std::array<std::vector<float3>, MAX_NUMBER_OF_TEXTURECOORDS> texcoords;
            std::vector<std::vector<std::pair<uint32_t, float>>> joint_bindings;

            std::vector<uint32_t> indices;
        };
        std::vector<Lod> lods;

        bool has_normal;
        bool has_tangent_frame;
        std::array<bool, MAX_NUMBER_OF_TEXTURECOORDS> has_texcoord;

        //AABBox pos_bb;
        //AABBox tc_bb;
    };


    std::vector<Mesh> meshes_;
    bool has_normal_;
	bool has_tangent_quat_;
	bool has_texcoord_;
	bool has_diffuse_;
	bool has_specular_;
};
}