#include <UI/GmDebugWindow.h>

#include <charconv>

#include <base/UIManager.h>
#include <RmlUi/Core/Input.h>
#include <base/ZEngine.h>
#include <common/Log.h>
#include <common/Util.h>
#include <game/GameContext.h>
#include <game/NpcSpawner.h>
#include <game/gas/CombatService.h>
#include <Manager/DataManager.h>
#include <render/Mesh.h>
#include <world/SceneNode.h>

using namespace RenderWorker;

namespace
{
constexpr char const* kDocPath = "rmlui/gm_debug.rml";
constexpr char const* kDocId = "GmDebugWindow";
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
	ui.SetKeyHandler([this](int key) {
		if (key == Rml::Input::KI_OEM_3) { ToggleVisible(); return true; }
		if (!visible_) return false;
		if (key == Rml::Input::KI_RETURN) { Submit(); return true; }
		if (key == Rml::Input::KI_ESCAPE) { SetVisible(false); return true; }
		return false;
	});
	return true;
}

void GmDebugWindow::Shutdown()
{
	if (document_)
	{
		Context::Instance().UIManagerInstance().SetKeyHandler({});
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

//createnpc 100000
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

	PrefabData const* pNpcData = GameContext::Instance().DataManagerInstance().FindNpc(npcId);
	if (pNpcData == nullptr)
	{
		LogInfo() << "[GM] npc id " << id_text << " find failed!" << std::endl;
		AppendLog("npc not found");
		return;
	}

	auto models = NpcSpawner::SpawnNpc(npcId);
	if (models.empty())
	{
		AppendLog("failed to load npc models");
		return;
	}

	for (RenderModelPtr const& model : models)
	{
		AddToSceneRootHelper(*model);
	}

	std::size_t expected_models = 0;
	for (ModelData const& component : pNpcData->components)
	{
		if (component.model && !component.model->model_path.empty())
		{
			++expected_models;
		}
	}

	AppendLog(
		std::string("spawned npc: ") + pNpcData->name + " (" + std::to_string(models.size()) + "/"
		+ std::to_string(expected_models) + " parts)");
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
