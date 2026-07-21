#include <UI/GmDebugWindow.h>

#include <charconv>

#include <base/UIManager.h>
#include <base/ZEngine.h>
#include <common/Log.h>
#include <game/GameContext.h>
#include <Manager/DataManager.h>
#include <game/Model.h>
#include <game/gas/CombatService.h>
#include <render/Mesh.h>
#include <render/RenderMaterial.h>
#include <render/Texture.h>
#include <render/RenderFactory.h>
#include <world/SceneNode.h>

using namespace RenderWorker;

namespace
{
constexpr char const* kDocPath = "rmlui/gm_debug.rml";
constexpr char const* kDocId = "GmDebugWindow";

void BindNpcTextureSlot(RenderMaterial& mtl, RenderMaterial::TextureSlot slot, std::string const& tex_path)
{
	if (tex_path.empty())
	{
		return;
	}

	mtl.TextureName(slot, tex_path);

	auto& context = Context::Instance();
	if (!context.RenderFactoryValid())
	{
		return;
	}

	auto& res_loader = context.ResLoaderInstance();
	if (res_loader.Locate(tex_path).empty() && res_loader.Locate(tex_path + ".dds").empty())
	{
		LogError() << "ApplyNpcMaterial: texture not found: " << tex_path << std::endl;
		return;
	}

	// Sync load so PS SRVs are HW-ready before the first draw (ASync leaves slots unbound until JIT finishes).
	auto& rf = context.RenderFactoryInstance();
	mtl.Texture(slot, rf.MakeTextureSrv(SyncLoadTexture(tex_path, EAH_GPU_Read | EAH_Immutable)));
}

void ApplyNpcMaterial(RenderModel& model, NpcData const& npc)
{
	bool const has_textures = !npc.textures.albedo.empty() || !npc.textures.metalness_glossiness.empty()
		|| !npc.textures.normal.empty();
	if (npc.material.empty() && !has_textures)
	{
		return;
	}

	for (size_t i = 0; i < model.NumMaterials(); ++i)
	{
		RenderMaterialPtr& mtl = model.GetMaterial(static_cast<int32_t>(i));
		if (!mtl)
		{
			continue;
		}

		if (!npc.material.empty())
		{
			mtl->Name(npc.material);
		}
		BindNpcTextureSlot(*mtl, RenderMaterial::TS_Albedo, npc.textures.albedo);
		BindNpcTextureSlot(*mtl, RenderMaterial::TS_MetalnessGlossiness, npc.textures.metalness_glossiness);
		BindNpcTextureSlot(*mtl, RenderMaterial::TS_Normal, npc.textures.normal);
	}
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
