#include "MeshLoader.h"

#include <core/IContext.h>

#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/material.h>
#include <assimp/GltfMaterial.h>

namespace CoreWorker
{
float3 Color4ToFloat3(aiColor4D const & c)
{
    float3 v;
    v.x() = c.r;
    v.y() = c.g;
    v.z() = c.b;
    return v;
}

MeshLoader::MeshLoader(IRenderModel* model)
    :render_model_(model)
{

}

void MeshLoader::LoadFromAssimp(std::string_view input_name, MeshMetadata const & metadata)
{
    unsigned int ppsteps = aiProcess_JoinIdenticalVertices // join identical vertices/ optimize indexing
        | aiProcess_ValidateDataStructure // perform a full validation of the loader's output
        | aiProcess_RemoveRedundantMaterials // remove redundant materials
        | aiProcess_FindInstances; // search for instanced meshes and remove them by references to one master

    uint32_t const num_lods = static_cast<uint32_t>(metadata.lod_file_names_.size());
    std::vector<Assimp::Importer> importers(num_lods);
	std::vector<aiScene const*> scenes(num_lods);
	for (uint32_t lod = 0; lod < num_lods; ++ lod)
    {
        std::string_view const lod_file_name = (lod == 0) ? input_name : metadata.lod_file_names_[lod];
		std::string const file_name = (lod == 0) ? std::string(input_name) : Context::Instance()->Locate(lod_file_name);
		if (file_name.empty())
		{ 
			LOGER_ERROR() << "Could NOT find " << lod_file_name << " for LoD " << lod << '.' << std::endl;
			return;
		}

        auto& importer = importers[lod];
        importer.SetPropertyInteger(AI_CONFIG_IMPORT_TER_MAKE_UVS, 1);
        importer.SetPropertyFloat(AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, 80);
        importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, 0);
        importer.SetPropertyInteger(AI_CONFIG_GLOB_MEASURE_TIME, 1);
			
        scenes[lod] = importer.ReadFile(file_name.c_str(),
            ppsteps // configurable pp steps
            | aiProcess_Triangulate // triangulate polygons with more than 3 edges
            | aiProcess_ConvertToLeftHanded // convert everything to D3D left handed space
            /*| aiProcess_FixInfacingNormals*/);

        if (!scenes[lod])
        {
            LOGER_ERROR() << "Assimp: Import file " << lod_file_name << " error: " << importer.GetErrorString() << std::endl;
            return;
        }
    }


    meshes_.resize(scenes[0]->mNumMeshes);
    for (size_t mi = 0; mi < meshes_.size(); ++ mi)
    {
        meshes_[mi].lods.resize(num_lods);
    }
    BuildMeshData(scenes);
    
    for (uint32_t lod = 0; lod < num_lods; ++ lod)
    {
        //this->BuildNodeData(num_lods, lod, -1, scenes[lod]->mRootNode);
    }

    BuildMaterials(scenes[0]);
}

void MeshLoader::BuildMeshData(std::vector<aiScene const*> const & scene_lods)
{
    for (size_t lod = 0; lod < scene_lods.size(); ++ lod)
    {
        for (unsigned int mi = 0; mi < scene_lods[lod]->mNumMeshes; ++ mi)
        {
            const aiMesh* mesh = scene_lods[lod]->mMeshes[mi];
            if (lod == 0)
            {
                meshes_[mi].mtl_id = mesh->mMaterialIndex;
                meshes_[mi].name = mesh->mName.C_Str();
            }

            auto& indices = meshes_[mi].lods[lod].indices;
            indices.resize(mesh->mNumFaces * 3);
            for (unsigned int fi = 0; fi < mesh->mNumFaces; ++ fi)
            {
                COMMON_ASSERT(3 == mesh->mFaces[fi].mNumIndices);
                for (uint32_t vi = 0; vi < 3; ++vi)
                {
                    indices[fi * 3 + vi] = mesh->mFaces[fi].mIndices[vi];
                }
            }

            auto& positions = meshes_[mi].lods[lod].positions;
            positions.resize(mesh->mNumVertices);

            bool has_normal = (mesh->mNormals != nullptr);
            auto& normals = meshes_[mi].lods[lod].normals;
            if (has_normal)
            {
                normals.resize(mesh->mNumVertices);
            }

            bool has_tangent = (mesh->mTangents != nullptr);
            auto& tangents = meshes_[mi].lods[lod].tangents;
            if (has_tangent)
            {
                tangents.resize(mesh->mNumVertices);
            }
            
            bool has_binormal = (mesh->mBitangents != nullptr);
            auto& binormals = meshes_[mi].lods[lod].binormals;
            if (has_binormal)
            {
                binormals.resize(mesh->mNumVertices);
            }

            auto& has_texcoord = meshes_[mi].has_texcoord;
            auto& texcoords = meshes_[mi].lods[lod].texcoords;
            uint32_t first_texcoord = AI_MAX_NUMBER_OF_TEXTURECOORDS;
            for (unsigned int tci = 0; tci < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++ tci)
            {
                has_texcoord[tci] = (mesh->mTextureCoords[tci] != nullptr);
                if (has_texcoord[tci])
                {
                    texcoords[tci].resize(mesh->mNumVertices);

                    if (first_texcoord == AI_MAX_NUMBER_OF_TEXTURECOORDS)
                    {
                        first_texcoord = tci;
                    }
                }
            }

            for (unsigned int vi = 0; vi < mesh->mNumVertices; ++ vi)
            {
                positions[vi] = float3(&mesh->mVertices[vi].x);

                if (has_normal)
                {
                    normals[vi] = float3(&mesh->mNormals[vi].x);
                }
                if (has_tangent)
                {
                    tangents[vi] = float3(&mesh->mTangents[vi].x);
                }
                if (has_binormal)
                {
                    binormals[vi] = float3(&mesh->mBitangents[vi].x);
                }

                for (unsigned int tci = 0; tci < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++ tci)
                {
                    if (has_texcoord[tci])
                    {
                        COMMON_ASSERT(mesh->mTextureCoords[tci] != nullptr);
                        COMMON_ASSUME(mesh->mTextureCoords[tci] != nullptr);

                        texcoords[tci][vi] = float3(&mesh->mTextureCoords[tci][vi].x);
                    }
                }
            }

            if (!has_normal)
            {
                normals.resize(mesh->mNumVertices);
                //MathLib::compute_normal(normals.begin(), indices.begin(), indices.end(), positions.begin(), positions.end());

                has_normal = true;
            }

            if ((!has_tangent || !has_binormal) && (first_texcoord != AI_MAX_NUMBER_OF_TEXTURECOORDS))
            {
                tangents.resize(mesh->mNumVertices);
                binormals.resize(mesh->mNumVertices);
                //MathLib::compute_tangent(tangents.begin(), binormals.begin(), indices.begin(), indices.end(),
                    //positions.begin(), positions.end(), texcoords[first_texcoord].begin(), normals.begin());

                has_tangent = true;
                has_binormal = true;
            }

            meshes_[mi].has_normal = has_normal;
            meshes_[mi].has_tangent_frame = has_tangent || has_binormal;

            if (meshes_[mi].has_tangent_frame)
            {
                has_tangent_quat_ = true;
            }
            else if (meshes_[mi].has_normal)
            {
                has_normal = true;
            }
            if (first_texcoord != AI_MAX_NUMBER_OF_TEXTURECOORDS)
            {
                has_texcoord_ = true;
            }

            if (mesh->mNumBones > 0)
            {
                meshes_[mi].lods[lod].joint_bindings.resize(mesh->mNumVertices);
                for (unsigned int bi = 0; bi < mesh->mNumBones; ++ bi)
                {
                    aiBone* bone = mesh->mBones[bi];
                    bool found = false;
                    // for (uint32_t ji = 0; ji < joints_.size(); ++ ji)
                    // {
                    //     std::string joint_name;
                    //     Convert(joint_name, joints_[ji].name);
                    //     if (joint_name == bone->mName.C_Str())
                    //     {
                    //         for (unsigned int wi = 0; wi < bone->mNumWeights; ++ wi)
                    //         {
                    //             float const weight = bone->mWeights[wi].mWeight;
                    //             if (weight >= 0.5f / 255)
                    //             {
                    //                 int const vertex_id = bone->mWeights[wi].mVertexId;
                    //                 meshes_[mi].lods[lod].joint_bindings[vertex_id].emplace_back(ji, weight);
                    //             }
                    //         }

                    //         found = true;
                    //         break;
                    //     }
                    // }
                    if (!found)
                    {
                        LOGER_ERROR() << (false, "Joint not found!");
                    }
                }

                for (auto& binding : meshes_[mi].lods[lod].joint_bindings)
                {
                    std::sort(binding.begin(), binding.end(),
                        [](std::pair<uint32_t, float> const & lhs, std::pair<uint32_t, float> const & rhs)
                        {
                            return lhs.second > rhs.second;
                        });
                }
            }
        }

        for (unsigned int mi = 0; mi < scene_lods[lod]->mNumMeshes; ++ mi)
        {
            if (has_tangent_quat_ && !meshes_[mi].has_tangent_frame)
            {
                meshes_[mi].has_tangent_frame = true;
            }
            if (has_texcoord_ && !meshes_[mi].has_texcoord[0])
            {
                meshes_[mi].has_texcoord[0] = true;
            }
        }
    }
}

void MeshLoader::BuildMaterials(aiScene const * scene)
{
    render_model_->NumMaterial(scene->mNumMaterials);
	for (unsigned int mi = 0; mi < scene->mNumMaterials; ++ mi)
	{
        std::string name;
        float3 albedo(0, 0, 0);
        float metalness = 0;
        float shininess = 1;
        float3 emissive(0, 0, 0);
        float opacity = 1;
        bool transparent = false;
        bool two_sided = false;
        float alpha_test = 0;

        aiString ai_name;
        aiColor4D ai_albedo(0, 0, 0, 0);
        float ai_opacity = 1;
        float ai_metallic = 0;
        float ai_shininess = 1;
        aiColor4D ai_emissive(0, 0, 0, 0);
        int ai_two_sided = 0;
        float ai_alpha_test = 0;

        auto mtl = scene->mMaterials[mi];

        if (AI_SUCCESS == aiGetMaterialString(mtl, AI_MATKEY_NAME, &ai_name))
        {
            name = ai_name.C_Str();
        }

        if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_BASE_COLOR, &ai_albedo))
        {
            albedo = Color4ToFloat3(ai_albedo);
            opacity = ai_albedo.a;
        }
        else
        {
            if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &ai_albedo))
            {
                albedo = Color4ToFloat3(ai_albedo);
            }
            if (AI_SUCCESS == aiGetMaterialFloat(mtl, AI_MATKEY_OPACITY, &ai_opacity))
            {
                opacity = ai_opacity;
            }
        }

        if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &ai_emissive))
        {
            emissive = Color4ToFloat3(ai_emissive);
        }

        if (AI_SUCCESS == aiGetMaterialFloat(mtl, AI_MATKEY_METALLIC_FACTOR, &ai_metallic))
        {
            metalness = ai_metallic;
        }

        if (AI_SUCCESS == aiGetMaterialFloat(mtl, AI_MATKEY_SHININESS, &ai_shininess))
			{
				shininess = ai_shininess;
			}
			shininess = MathWorker::clamp(shininess, 1.0f, MAX_SHININESS);

			if ((opacity < 1) || (aiGetMaterialTextureCount(mtl, aiTextureType_OPACITY) > 0))
			{
				transparent = true;
			}

			if (AI_SUCCESS == aiGetMaterialInteger(mtl, AI_MATKEY_TWOSIDED, &ai_two_sided))
			{
				two_sided = ai_two_sided ? true : false;
			}

			aiString ai_alpha_mode;
			if (AI_SUCCESS == aiGetMaterialString(mtl, AI_MATKEY_GLTF_ALPHAMODE, &ai_alpha_mode))
			{
				if (strcmp(ai_alpha_mode.C_Str(), "MASK") == 0)
				{
					if (AI_SUCCESS == aiGetMaterialFloat(mtl, AI_MATKEY_GLTF_ALPHACUTOFF, &ai_alpha_test))
					{
						alpha_test = ai_alpha_test;
					}
				}
				else if (strcmp(ai_alpha_mode.C_Str(), "BLEND") == 0)
				{
					transparent = true;
				}
				else if(strcmp(ai_alpha_mode.C_Str(), "OPAQUE") == 0)
				{
					transparent = false;
				}
			}

			render_model_->GetMaterial(mi) = CommonWorker::MakeSharedPtr<IRenderMaterial>();
			auto& render_mtl = *render_model_->GetMaterial(mi);
			render_mtl.SetName(name);
			render_mtl.Albedo(float4(albedo.x(), albedo.y(), albedo.z(), opacity));
			render_mtl.Metalness(metalness);
			render_mtl.Glossiness(Shininess2Glossiness(shininess));
			render_mtl.Emissive(emissive);
			render_mtl.Transparent(transparent);
			render_mtl.AlphaTestThreshold(alpha_test);
			render_mtl.SSS(false);
			render_mtl.TwoSided(two_sided);

			unsigned int count = aiGetMaterialTextureCount(mtl, aiTextureType_DIFFUSE);
			if (count > 0)
			{
				aiString str;
				aiGetMaterialTexture(mtl, aiTextureType_DIFFUSE, 0, &str, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
				render_mtl.SetTextureName(IRenderMaterial::TS_Albedo, str.C_Str());
			}

			count = aiGetMaterialTextureCount(mtl, aiTextureType_UNKNOWN);
			if (count > 0)
			{
				aiString str;
				aiGetMaterialTexture(mtl, AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, &str, nullptr, nullptr, nullptr,
					nullptr, nullptr, nullptr);
				render_mtl.SetTextureName(IRenderMaterial::TS_MetalnessGlossiness, str.C_Str());
			}
			else
			{
				count = aiGetMaterialTextureCount(mtl, aiTextureType_SHININESS);
				if (count > 0)
				{
					aiString str;
					aiGetMaterialTexture(mtl, aiTextureType_SHININESS, 0, &str, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
					render_mtl.SetTextureName(IRenderMaterial::TS_MetalnessGlossiness, str.C_Str());
				}
			}

			count = aiGetMaterialTextureCount(mtl, aiTextureType_EMISSIVE);
			if (count > 0)
			{
				aiString str;
				aiGetMaterialTexture(mtl, aiTextureType_EMISSIVE, 0, &str, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
				render_mtl.SetTextureName(IRenderMaterial::TS_Emissive, str.C_Str());
			}

			count = aiGetMaterialTextureCount(mtl, aiTextureType_NORMALS);
			if (count > 0)
			{
				aiString str;
				aiGetMaterialTexture(mtl, aiTextureType_NORMALS, 0, &str, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
				render_mtl.SetTextureName(IRenderMaterial::TS_Normal, str.C_Str());

				float normal_scale;
				aiGetMaterialFloat(mtl, AI_MATKEY_GLTF_TEXTURE_SCALE(aiTextureType_NORMALS, 0), &normal_scale);
				render_mtl.NormalScale(normal_scale);
			}

			count = aiGetMaterialTextureCount(mtl, aiTextureType_HEIGHT);
			if (count > 0)
			{
				aiString str;
				aiGetMaterialTexture(mtl, aiTextureType_HEIGHT, 0, &str, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
				render_mtl.SetTextureName(IRenderMaterial::TS_Height, str.C_Str());
			}

			count = aiGetMaterialTextureCount(mtl, aiTextureType_LIGHTMAP);
			if (count > 0)
			{
				aiString str;
				aiGetMaterialTexture(mtl, aiTextureType_LIGHTMAP, 0, &str, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
				render_mtl.SetTextureName(IRenderMaterial::TS_Occlusion, str.C_Str());

				float occlusion_strength;
				aiGetMaterialFloat(mtl, AI_MATKEY_GLTF_TEXTURE_STRENGTH(aiTextureType_LIGHTMAP, 0), &occlusion_strength);
				render_mtl.OcclusionStrength(occlusion_strength);
			}

			render_mtl.DetailMode(IRenderMaterial::SurfaceDetailMode::ParallaxMapping);
			if (render_mtl.GetTextureName(IRenderMaterial::TS_Height).empty())
			{
				render_mtl.HeightOffset(0);
				render_mtl.HeightScale(0);
			}
			else
			{
				render_mtl.HeightOffset(-0.5f);
				render_mtl.HeightScale(0.06f);

				float ai_bumpscaling;
				if (AI_SUCCESS == aiGetMaterialFloat(mtl, AI_MATKEY_BUMPSCALING, &ai_bumpscaling))
				{
					render_mtl.HeightScale(ai_bumpscaling);
				}
			}
			render_mtl.EdgeTessHint(5);
			render_mtl.InsideTessHint(5);
			render_mtl.MinTessFactor(1);
			render_mtl.MaxTessFactor(9);
    }
}
}