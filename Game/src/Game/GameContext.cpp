#include <game/GameContext.h>

#include <base/UIManager.h>
#include <base/ZEngine.h>
#include <common/Log.h>

GameContext& GameContext::Instance()
{
	static GameContext instance;
	return instance;
}

void GameContext::Startup()
{
	if (started_)
	{
		return;
	}

	if (!data_manager_.LoadNpcConfig())
	{
		CommonWorker::LogError() << "GameContext: failed to load NPC config." << std::endl;
	}

	// GmDebugWindow needs UIManager dimensions set by the app after Create/OnResize.
	started_ = true;
}

void GameContext::Shutdown()
{
	if (!started_)
	{
		return;
	}

	gm_debug_window_.Shutdown();
	data_manager_.Clear();
	started_ = false;
}

DataManager& GameContext::DataManagerInstance() noexcept
{
	return data_manager_;
}

DataManager const& GameContext::DataManagerInstance() const noexcept
{
	return data_manager_;
}

GmDebugWindow& GameContext::GmDebugWindowInstance() noexcept
{
	return gm_debug_window_;
}

GmDebugWindow const& GameContext::GmDebugWindowInstance() const noexcept
{
	return gm_debug_window_;
}
