#pragma once

#include <game/GameApi.h>
#include <render/Mesh.h>
#include <base/ZEngine.h>

class GAME_API DetailedMesh : public RenderWorker::StaticMesh
{
public:
	explicit DetailedMesh(std::wstring_view name);

	void OnRenderBegin();

	void EyePos(RenderWorker::float3 const& eye_pos);
	void LightPos(RenderWorker::float3 const& light_pos);
	void LightColor(RenderWorker::float3 const& light_color);
	void LightFalloff(RenderWorker::float3 const& light_falloff);

	void BackFaceDepthPass(bool dfdp);
	void BackFaceDepthTex(RenderWorker::TexturePtr const& tex);
	void SigmaT(float sigma_t);
	void MtlThickness(float thickness);

protected:
	void DoBuildMeshInfo(RenderWorker::RenderModel const& model) override;

private:
	bool depth_texture_support_;
};

class GAME_API AModel : public RenderWorker::RenderModel
{
public:
	explicit AModel(const SceneNodePtr& root_node);
	AModel(std::wstring_view name, uint32_t node_attrib);

	void BuildModelInfo() override;
};
using ModelPtr = std::shared_ptr<AModel>;

GAME_API RenderWorker::StaticMeshPtr CreateDetailedMesh(std::wstring_view name);
GAME_API RenderWorker::RenderModelPtr CreateGameModel(std::wstring_view name, uint32_t node_attrib);
