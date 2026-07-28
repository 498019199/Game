#include <UI/GmDebugWindow.h>

#include <algorithm>
#include <cctype>
#include <charconv>
#include <unordered_set>

#include <base/UIManager.h>
#include <base/ZEngine.h>
#include <common/Log.h>
#include <common/Util.h>
#include <game/GameContext.h>
#include <Manager/DataManager.h>
#include <game/Model.h>
#include <game/gas/CombatService.h>
#include <render/Mesh.h>
#include <render/RenderEffect.h>
#include <render/RenderMaterial.h>
#include <render/Texture.h>
#include <render/RenderFactory.h>
#include <world/SceneNode.h>

using namespace RenderWorker;

namespace
{
constexpr char const* kDocPath = "rmlui/gm_debug.rml";
constexpr char const* kDocId = "GmDebugWindow";

	bool HasAnyTexture(NpcTextures const& textures)
	{
		return !textures.albedo.empty() || !textures.metalness_glossiness.empty() || !textures.normal.empty()
			|| !textures.emissive.empty() || !textures.detail.empty() || !textures.detail2.empty()
			|| !textures.detail_mask.empty() || !textures.cubemap.empty() || !textures.translucency.empty()
			|| !textures.mask1.empty() || !textures.mask2.empty() || !textures.diffuse_warp.empty()
			|| !textures.fresnel_warp_color.empty() || !textures.fresnel_warp_rim.empty()
			|| !textures.fresnel_warp_spec.empty();
	}

std::string ToLowerAscii(std::string value)
{
	std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
		return static_cast<char>(std::tolower(ch));
	});
	return value;
}

// color.tga / _color.TGA -> sibling map name (metalnessMask, selfIllumMask, ...).
std::string DeriveColorSibling(std::string const& albedo_path, std::string_view suffix)
{
	if (albedo_path.empty() || suffix.empty())
	{
		return {};
	}

	std::string const lower = ToLowerAscii(albedo_path);
	std::string_view const tokens[] = {"_color.", "-color.", "color."};
	for (std::string_view token : tokens)
	{
		auto const pos = lower.rfind(token);
		if (pos == std::string::npos)
		{
			continue;
		}
		std::string out = albedo_path;
		std::string replace(suffix);
		replace.append(token.substr(token.size() - 1)); // keep '.'
		out.replace(pos, token.size(), replace);
		return out;
	}
	return {};
}

std::string ResolveExistingTexturePath(std::string const& configured, std::string const& albedo, std::string_view sibling_suffix)
{
	auto& res_loader = Context::Instance().ResLoaderInstance();
	auto exists = [&](std::string const& path) {
		return !path.empty()
			&& (!res_loader.Locate(path).empty() || !res_loader.Locate(path + ".dds").empty());
	};

	if (exists(configured))
	{
		return configured;
	}
	std::string const derived = DeriveColorSibling(albedo, sibling_suffix);
	if (exists(derived))
	{
		return derived;
	}
	return configured.empty() ? derived : configured;
}

	NpcTextures ResolveNpcTextures(NpcTextures const& src)
	{
		NpcTextures out = src;
		out.mask1 = ResolveExistingTexturePath(src.mask1, src.albedo, "mask1");
		out.mask2 = ResolveExistingTexturePath(src.mask2, src.albedo, "mask2");
		out.detail = ResolveExistingTexturePath(src.detail, src.albedo, "detail");
		out.detail2 = ResolveExistingTexturePath(src.detail2, src.albedo, "detail2");
		out.cubemap = ResolveExistingTexturePath(src.cubemap, src.albedo, "cubeMap");
		out.diffuse_warp = ResolveExistingTexturePath(src.diffuse_warp, src.albedo, "diffuseWarp");
		out.fresnel_warp_color = ResolveExistingTexturePath(src.fresnel_warp_color, src.albedo, "fresnelWarpColor");
		out.fresnel_warp_rim = ResolveExistingTexturePath(src.fresnel_warp_rim, src.albedo, "fresnelWarpRim");
		out.fresnel_warp_spec = ResolveExistingTexturePath(src.fresnel_warp_spec, src.albedo, "fresnelWarpSpec");
		if (out.normal.empty())
		{
			out.normal = ResolveExistingTexturePath({}, src.albedo, "normal");
		}
		// Packed workshop maps absorb metalness/selfIllum/detailMask/translucency.
		if (out.mask1.empty())
		{
			out.metalness_glossiness =
				ResolveExistingTexturePath(src.metalness_glossiness, src.albedo, "metalnessMask");
			out.emissive = ResolveExistingTexturePath(src.emissive, src.albedo, "selfIllumMask");
			out.detail_mask = ResolveExistingTexturePath(src.detail_mask, src.albedo, "detailMask");
			out.translucency = ResolveExistingTexturePath(src.translucency, src.albedo, "translucency");
		}
		else
		{
			out.metalness_glossiness.clear();
			out.emissive.clear();
			out.detail_mask.clear();
			out.translucency.clear();
		}
		return out;
	}

ShaderResourceViewPtr LoadNpcTextureSrv(std::string const& tex_path)
{
	if (tex_path.empty())
	{
		return {};
	}

	auto& context = Context::Instance();
	if (!context.RenderFactoryValid())
	{
		return {};
	}

	auto& res_loader = context.ResLoaderInstance();
	if (res_loader.Locate(tex_path).empty() && res_loader.Locate(tex_path + ".dds").empty())
	{
		LogError() << "ApplyNpcMaterial: texture not found: " << tex_path << std::endl;
		return {};
	}

	auto& rf = context.RenderFactoryInstance();
	auto tex = SyncLoadTexture(tex_path, EAH_GPU_Read | EAH_Immutable);
	if (!tex)
	{
		LogError() << "ApplyNpcMaterial: SyncLoadTexture failed: " << tex_path << std::endl;
		return {};
	}
	auto srv = rf.MakeTextureSrv(tex);
	if (!srv)
	{
		LogError() << "ApplyNpcMaterial: MakeTextureSrv failed: " << tex_path << std::endl;
	}
	return srv;
}

void BindNpcTextureSlot(RenderMaterial& mtl, RenderMaterial::TextureSlot slot, std::string const& tex_path)
{
	if (tex_path.empty())
	{
		return;
	}

	mtl.TextureName(slot, tex_path);
	if (auto srv = LoadNpcTextureSrv(tex_path))
	{
		mtl.Texture(slot, std::move(srv));
	}
}

void SetEffectTextureParam(RenderEffect& effect, char const* name, std::string const& tex_path, char const* enabled_name)
{
	auto* param = effect.ParameterByName(name);
	auto* enabled = enabled_name ? effect.ParameterByName(enabled_name) : nullptr;
	if (!param)
	{
		return;
	}

	if (auto srv = LoadNpcTextureSrv(tex_path))
	{
		*param = srv;
		if (enabled)
		{
			*enabled = 1;
		}
	}
	else if (enabled)
	{
		*enabled = 0;
	}
}

void ApplyTexturesToMaterial(RenderMaterial& mtl, std::string const& material_name, NpcTextures const& textures_in)
{
	NpcTextures const textures = ResolveNpcTextures(textures_in);
	bool const has_textures = HasAnyTexture(textures);
	if (material_name.empty() && !has_textures)
	{
		return;
	}

	if (!material_name.empty())
	{
		mtl.Name(material_name);
	}

	// FBX/MIC often leave diffuse/base color at 0; albedo *= map, so black tint kills all color.
	if (has_textures)
	{
		mtl.Albedo(float4(1.0f, 1.0f, 1.0f, 1.0f));
		if (mtl.Glossiness() <= 0.0f)
		{
			mtl.Glossiness(0.5f);
		}
	}

		BindNpcTextureSlot(mtl, RenderMaterial::TS_Albedo, textures.albedo);
		BindNpcTextureSlot(mtl, RenderMaterial::TS_MetalnessGlossiness, textures.metalness_glossiness);
		if ((!textures.metalness_glossiness.empty() || !textures.mask1.empty()) && mtl.Metalness() <= 0.0f)
		{
			// metalness scales factor.x; keep base at 1 so the mask is visible.
			mtl.Metalness(1.0f);
		}
		BindNpcTextureSlot(mtl, RenderMaterial::TS_Normal, textures.normal);
		BindNpcTextureSlot(mtl, RenderMaterial::TS_Emissive, textures.emissive);
	}

	void ApplyData2ExtraTextures(RenderEffect& effect, NpcTextures const& textures_in)
	{
		NpcTextures const textures = ResolveNpcTextures(textures_in);
		SetEffectTextureParam(effect, "mask1_tex", textures.mask1, "mask1_map_enabled");
		SetEffectTextureParam(effect, "mask2_tex", textures.mask2, "mask2_map_enabled");
		SetEffectTextureParam(effect, "detail_tex", textures.detail, "detail_map_enabled");
		SetEffectTextureParam(effect, "detail2_tex", textures.detail2, "detail2_map_enabled");
		SetEffectTextureParam(effect, "detail_mask_tex", textures.detail_mask, "detail_mask_enabled");
		bool const has_detail_mask = !textures.mask1.empty() || !textures.detail_mask.empty();
		// detail1 requires mask; detail2 can run without mask (dmask=1).
		if (textures.detail.empty() || !has_detail_mask)
		{
			if (auto* enabled = effect.ParameterByName("detail_map_enabled"))
			{
				*enabled = 0;
			}
		}
		if (textures.detail2.empty())
		{
			if (auto* enabled = effect.ParameterByName("detail2_map_enabled"))
			{
				*enabled = 0;
			}
		}
		SetEffectTextureParam(effect, "cubemap_tex", textures.cubemap, "cubemap_map_enabled");
		SetEffectTextureParam(effect, "translucency_tex", textures.translucency, "translucency_map_enabled");
		SetEffectTextureParam(effect, "diffuse_warp_tex", textures.diffuse_warp, "diffuse_warp_enabled");
		SetEffectTextureParam(effect, "fresnel_warp_color_tex", textures.fresnel_warp_color, "fresnel_warp_color_enabled");
		SetEffectTextureParam(effect, "fresnel_warp_rim_tex", textures.fresnel_warp_rim, "fresnel_warp_rim_enabled");
		SetEffectTextureParam(effect, "fresnel_warp_spec_tex", textures.fresnel_warp_spec, "fresnel_warp_spec_enabled");
		if (auto* selfillum = effect.ParameterByName("selfillum_map_enabled"))
		{
			*selfillum = (!textures.mask1.empty() || !textures.emissive.empty()) ? 1 : 0;
		}

		if (auto* p = effect.ParameterByName("debug_mask1_raw"))
		{
			*p = 2.0f;
		}
	}

NpcPart const* FindPartForMeshName(std::string const& mesh_name_lower, std::vector<NpcPart> const& parts)
{
	for (NpcPart const& part : parts)
	{
		if (part.name.empty())
		{
			continue;
		}
		std::string const key = ToLowerAscii(part.name);
		if (mesh_name_lower.find(key) != std::string::npos)
		{
			return &part;
		}
	}
	return nullptr;
}

void ApplyNpcMaterial(RenderModel& model, NpcData const& npc)
{
	bool const has_parts = !npc.parts.empty();
	bool const has_fallback = !npc.material.empty() || HasAnyTexture(npc.textures);
	if (!has_parts && !has_fallback)
	{
		return;
	}

	// No parts: keep legacy behavior — paint every material with top-level textures.
	if (!has_parts)
	{
		for (size_t i = 0; i < model.NumMaterials(); ++i)
		{
			RenderMaterialPtr& mtl = model.GetMaterial(static_cast<int32_t>(i));
			if (!mtl)
			{
				continue;
			}
			ApplyTexturesToMaterial(*mtl, npc.material, npc.textures);
		}
		return;
	}

		std::unordered_set<int32_t> covered_materials;
		for (uint32_t mesh_index = 0; mesh_index < model.NumMeshes(); ++mesh_index)
		{
			auto& mesh = CommonWorker::checked_cast<StaticMesh&>(*model.Mesh(mesh_index));
			std::string mesh_name;
			CommonWorker::Convert(mesh_name, mesh.Name());
			std::string const mesh_name_lower = ToLowerAscii(mesh_name);

			NpcPart const* part = FindPartForMeshName(mesh_name_lower, npc.parts);
			if (!part)
			{
				LogInfo() << "ApplyNpcMaterial: unmatched mesh '" << mesh_name << "' for npc " << npc.name
						  << std::endl;
				continue;
			}

			int32_t const mtl_id = mesh.MaterialID();
			if ((mtl_id < 0) || (static_cast<size_t>(mtl_id) >= model.NumMaterials()))
			{
				LogError() << "ApplyNpcMaterial: invalid MaterialID " << mtl_id << " on mesh '" << mesh_name
						   << "'" << std::endl;
				continue;
			}

			RenderMaterialPtr& mtl = model.GetMaterial(mtl_id);
			if (!mtl)
			{
				continue;
			}

			ApplyTexturesToMaterial(*mtl, "", part->textures);
			covered_materials.insert(mtl_id);
		}

		// Uncovered materials: top-level textures, else first part (parts-only NPCs like chaos_knight).
		NpcTextures const* fallback_tex = nullptr;
		std::string fallback_mtl_name;
		if (has_fallback)
		{
			fallback_tex = &npc.textures;
			fallback_mtl_name = npc.material;
		}
		else if (!npc.parts.empty())
		{
			fallback_tex = &npc.parts.front().textures;
		}
		if (!fallback_tex)
		{
			return;
		}

		for (size_t i = 0; i < model.NumMaterials(); ++i)
		{
			if (covered_materials.contains(static_cast<int32_t>(i)))
			{
				continue;
			}

			RenderMaterialPtr& mtl = model.GetMaterial(static_cast<int32_t>(i));
			if (!mtl)
			{
				continue;
			}
			ApplyTexturesToMaterial(*mtl, fallback_mtl_name, *fallback_tex);
		}
}

void ApplyNpcRenderEffect(RenderModel& model, NpcData const& npc)
{
	if (npc.render_effect.empty())
	{
		return;
	}

	RenderEffectPtr effect_template = SyncLoadRenderEffect(npc.render_effect);
	if (!effect_template)
	{
		LogError() << "ApplyNpcRenderEffect: failed to load effect '" << npc.render_effect << "' for npc "
				   << npc.name << std::endl;
		return;
	}

	if (npc.render_technique.empty())
	{
		LogError() << "ApplyNpcRenderEffect: render_technique is empty for npc " << npc.name << std::endl;
		return;
	}

	if (!effect_template->TechniqueByName(npc.render_technique))
	{
		LogError() << "ApplyNpcRenderEffect: technique '" << npc.render_technique << "' not found in '"
				   << npc.render_effect << "' for npc " << npc.name << std::endl;
		return;
	}

	NpcTextures const* fallback_tex = HasAnyTexture(npc.textures) ? &npc.textures
		: (!npc.parts.empty() ? &npc.parts.front().textures : nullptr);

	model.ForEachMesh([&](Renderable& mesh) {
		auto& static_mesh = CommonWorker::checked_cast<StaticMesh&>(mesh);
		RenderEffectPtr mesh_effect = effect_template->Clone();
		RenderTechnique* tech = mesh_effect->TechniqueByName(npc.render_technique);
		static_mesh.Technique(mesh_effect, tech);

		NpcTextures const* tex = fallback_tex;
		std::string mesh_name;
		CommonWorker::Convert(mesh_name, static_mesh.Name());
		if (NpcPart const* part = FindPartForMeshName(ToLowerAscii(mesh_name), npc.parts))
		{
			tex = &part->textures;
		}
		if (tex)
		{
			ApplyData2ExtraTextures(*mesh_effect, *tex);
		}
	});
}

}

GmDebugWindow::GmDebugWindow() = default;

GmDebugWindow::~GmDebugWindow()
{
	Shutdown();
}

bool GmDebugWindow::Initialize()
{
	if (initialized_)
	{
		return document_ != nullptr;
	}

	auto& ui = Context::Instance().UIManagerInstance();
	if (!ui.Valid())
	{
		LogError() << "GmDebugWindow: UIManager not ready." << std::endl;
		return false;
	}

	document_ = ui.GetDocument(kDocId);
	if (!document_)
	{
		document_ = ui.LoadDocument(kDocPath);
	}
	if (!document_)
	{
		LogError() << "GmDebugWindow: failed to load " << kDocPath << std::endl;
		return false;
	}

	ui.HideDocument(document_);
	visible_ = false;
	initialized_ = true;
	return true;
}

void GmDebugWindow::Shutdown()
{
	if (document_)
	{
		Context::Instance().UIManagerInstance().CloseDocument(document_);
		document_ = nullptr;
	}
	visible_ = false;
	initialized_ = false;
}

void GmDebugWindow::ToggleVisible()
{
	SetVisible(!visible_);
}

void GmDebugWindow::SetVisible(bool visible)
{
	if (!initialized_ && !Initialize())
	{
		return;
	}
	if (!document_)
	{
		return;
	}

	auto& ui = Context::Instance().UIManagerInstance();
	visible_ = visible;
	if (visible_)
	{
		ui.ShowDocument(document_);
		ui.PullDocumentToFront(document_);
		FocusInput();
		LogInfo() << "GM console shown (press ~ to hide)." << std::endl;
	}
	else
	{
		ui.HideDocument(document_);
	}
}

void GmDebugWindow::FocusInput()
{
	if (!document_)
	{
		return;
	}
	Context::Instance().UIManagerInstance().FocusElement(document_, "gm_input");
}

void GmDebugWindow::Submit()
{
	if (!document_)
	{
		return;
	}

	auto& ui = Context::Instance().UIManagerInstance();
	std::string cmd = ui.GetInputValue(document_, "gm_input");
	ui.SetInputValue(document_, "gm_input", "");
	ExecuteCommand(cmd);
}

void GmDebugWindow::ExecuteCommand(std::string_view command)
{
	if (command.empty())
	{
		return;
	}

	LogInfo() << "[GM] " << command << std::endl;
	AppendLog(std::string("> ") + std::string(command));

	std::vector<std::string_view> strs = StringUtil::Split(command, StringUtil::EqualTo(' '));
	if (command == "help")
	{
		AppendLog("commands: help, clear, /createnpc <id>, /gas smoke, /gas skillsmoke");
	}
	else if (command == "clear")
	{
		if (document_)
		{
			Context::Instance().UIManagerInstance().SetInnerRml(document_, "gm_log", "");
		}
	}
	else if (strs[0] == "/additem")
	{

	}
	else if (strs[0] == "/createnpc")
	{
		CreateNpc(strs.size() >= 2 ? strs[1] : std::string_view {});
	}
	else if (strs[0] == "/gas")
	{
		if (strs.size() >= 2 && strs[1] == "smoke")
		{
			bool const ok = Gas::RunGasSmokeTest();
			AppendLog(ok ? "gas smoke: OK" : "gas smoke: FAIL (see log)");
		}
		else if (strs.size() >= 2 && strs[1] == "skillsmoke")
		{
			bool const ok = Gas::RunGasSkillConfigSmokeTest();
			AppendLog(ok ? "gas skillsmoke: OK" : "gas skillsmoke: FAIL (see log)");
		}
		else
		{
			AppendLog("usage: /gas smoke|skillsmoke");
		}
	}
	else
	{
		AppendLog(std::string("unknown: ") + std::string(command));
	}
}

//createnpc 100000 1
void GmDebugWindow::CreateNpc(std::string_view id_text)
{
	if (id_text.empty())
	{
		AppendLog("usage: /createnpc <id>");
		return;
	}

	int32_t npcId = 0;
	auto const [end, ec] = std::from_chars(id_text.data(), id_text.data() + id_text.size(), npcId);
	if (ec != std::errc{} || end != id_text.data() + id_text.size())
	{
		AppendLog("invalid npc id");
		return;
	}

	NpcData const* pNpcData = GameContext::Instance().DataManagerInstance().FindNpc(npcId);
	if (pNpcData == nullptr)
	{
		LogInfo() << "[GM] npc id " << id_text << " find failed!" << std::endl;
		AppendLog("npc not found");
		return;
	}

	if (pNpcData->models.empty())
	{
		AppendLog("npc has no models");
		return;
	}

	std::size_t loaded = 0;
	for (std::string const& model_path : pNpcData->models)
	{
		RenderModelPtr model = SyncLoadModel(
			model_path,
			EAH_GPU_Read | EAH_Immutable,
			SceneNode::SOA_Cullable,
			[pNpcData](RenderModel& loaded_model)
			{
				ApplyNpcMaterial(loaded_model, *pNpcData);
				ApplyNpcRenderEffect(loaded_model, *pNpcData);
				loaded_model.RootNode()->TransformToParent(MathWorker::translation(0.0f, 0.0f, 0.0f));
				AddToSceneRootHelper(loaded_model);
			},
			CreateGameModel,
			CreateDetailedMesh);
		if (!model)
		{
			AppendLog(std::string("failed to load npc model: ") + model_path);
			continue;
		}
		++loaded;
	}

	if (loaded == 0)
	{
		AppendLog("failed to load npc models");
		return;
	}

	AppendLog(
		std::string("spawned npc: ") + pNpcData->name + " (" + std::to_string(loaded) + "/"
		+ std::to_string(pNpcData->models.size()) + " parts)");
}

void GmDebugWindow::AppendLog(std::string_view text)
{
	if (!document_)
	{
		return;
	}

	auto& ui = Context::Instance().UIManagerInstance();
	std::string html = ui.GetInnerRml(document_, "gm_log");
	html += "<p>";
	html.append(text);
	html += "</p>";
	ui.SetInnerRml(document_, "gm_log", html);
}
