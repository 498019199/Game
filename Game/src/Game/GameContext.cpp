#include <game/GameContext.h>

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

	started_ = true;
}

void GameContext::Shutdown()
{
	if (!started_)
	{
		return;
	}

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
