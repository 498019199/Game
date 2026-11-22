#include <ui/GmDebugWindow.h>


#include <base/UIManager.h>
#include <RmlUi/Core/Input.h>
#include <base/ZEngine.h>
#include <common/Log.h>
#include <common/Util.h>
#include <game/GameContext.h>
#include <game/NpcSpawner.h>
#include <manager/DataManager.h>
#include <render/Mesh.h>
#include <world/SceneNode.h>

#include <charconv>
#include <sstream>

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
    if (command.empty()) return;
    LogInfo() << "[GM] " << command << std::endl;
    AppendLog(std::string("> ") + std::string(command));
    std::istringstream input{std::string(command)};
    std::string name;
    if (!(input >> name)) return;

    if (name == "help")
        AppendLog("commands: help, clear, /createnpc <id>");
    else if (name == "clear")
        Context::Instance().UIManagerInstance().SetInnerRml(document_, "gm_log", "");
    else if (name == "/createnpc")
    {
        std::string id_text;
        input >> id_text;
        int32_t id = 0;
        auto const end = id_text.data() + id_text.size();
        auto const [last, error] = std::from_chars(id_text.data(), end, id);
        if (error == std::errc{} && last == end)
            CreateNpc(id);
        else
            AppendLog("usage: /createnpc <id>");
    }
    else
        AppendLog("unknown: " + std::string(command));
}

//createnpc 100000
void GmDebugWindow::CreateNpc(int32_t npcId)
{
	PrefabData const* pNpcData = GameContext::Instance().DataManagerInstance().FindNpc(npcId);
	if (pNpcData == nullptr)
	{
		LogInfo() << "[GM] npc id " << npcId << " find failed!" << std::endl;
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
	for (char c : text)
    {
        switch (c)
        {
        case '&': html += "&amp;"; break;
        case '<': html += "&lt;"; break;
        case '>': html += "&gt;"; break;
        default: html += c; break;
        }
    }
	html += "</p>";
	ui.SetInnerRml(document_, "gm_log", html);
}
