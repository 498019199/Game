#include "test.h"
#include <base/ZEngine.h>
#include <base/App3D.h>
#include <base/Window.h>

namespace RenderWorker
{
class TestApp: public App3D
{
public:
	TestApp()
		:App3D("Tests")
	{
		Context::Instance().ResLoaderInstance().AddPath("../../tests/bin");
	}
	
    ~TestApp()
	{
	}

private :
    virtual uint32_t DoUpdate( [[maybe_unused]] uint32_t pass) override
	{
		return URV_Finished;
	}
};

class TestEnvironment : public testing::Environment
{
public:
	void SetUp() override
	{
		Context::Instance().LoadConfig("KlayGE.cfg");
		auto config = Context::Instance().Config();
		config.graphics_cfg.hide_win = true;
		config.graphics_cfg.hdr = false;
		config.graphics_cfg.color_grading = false;
		config.graphics_cfg.gamma = false;
		Context::Instance().Config(config);

		app_ = MakeUniquePtr<TestApp>();
		app_->Create();
	}

	void TearDown() override
	{
		app_.reset();

		Context::Destroy();
	}

private:
	std::unique_ptr<App3D> app_;
};
}

int main(int argc, char** argv)
{
	testing::InitGoogleTest(&argc, argv);
	AddGlobalTestEnvironment(new RenderWorker::TestEnvironment);

    int ret_val = RUN_ALL_TESTS();
	if (ret_val != 0)
	{
		[[maybe_unused]] int ch = getchar();
	}

	return ret_val;
}