#include "App.h"
#include "../Core/Context.h"
#include "../Platform/Window/Window.h"
#include "../Platform/Renderer.h"
App::App(const std::string& name, void* native_wnd)
	:m_strAppName(name), m_MainWinPtr(nullptr),
	m_fAppTime(0.f),m_fFrameTime(0.f), m_nTotalNumFrames(0),
	n_nNumFrame(0), m_fAccumulate(0.f), m_fFPS(0.f)
{
	Context::Instance()->AppInstance(this);

	auto cfg = Context::Instance()->GetConfig();
	m_MainWinPtr = this->MakeWindow(m_strAppName, cfg, native_wnd);
}

App::~App()
{
	this->Destroy();
}

void App::Create()
{
	auto config = Context::Instance()->GetConfig();
	this->OnCreate();
}

void App::Destroy()
{
	this->OnDestroy();
	EndCore();
}

void App::Run()
{
#if defined STX_PLATFORM_WINDOWS_DESKTOP
	bool gotMsg;
	MSG  msg;

	auto rf = Context::Instance()->GetSubsystem<Renderer>();
	::PeekMessage(&msg, nullptr, 0, 0, PM_NOREMOVE);
	while (WM_QUIT != msg.message)
	{
		// 如果窗口是激活的，用 PeekMessage()以便我们可以用空闲时间渲染场景
		// 不然, 用 GetMessage() 减少 CPU 占用率
		if (m_MainWinPtr->Active())
		{
			gotMsg = (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) != 0);
		}
		else
		{
			gotMsg = (::GetMessage(&msg, nullptr, 0, 0) != 0);
		}

		if (gotMsg)
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		else
		{
			rf->Refresh();
			this->Update(0);
		}
	}
#elif defined STX_PLATFORM_LINUX
	::Display* x_display = m_MainWinPtr->XDisplay();
	XEvent event;
	while (!m_MainWinPtr->Closed())
	{
		do
		{
			XNextEvent(x_display, &event);
			m_MainWinPtr->MsgProc(event);
		} while (XPending(x_display));

		re.Refresh();
		this->Update(0);
	}
#endif
}

void App::Quit()
{
#ifdef STX_PLATFORM_WINDOWS
#ifdef STX_PLATFORM_WINDOWS_DESKTOP
	::PostQuitMessage(0);
#endif
#else
	exit(0);
#endif
}

WindowPtr App::MakeWindow(const std::string& name, const WindowDesc& settings)
{
	return MakeSharedPtr<Window>(name, settings, nullptr);
}

WindowPtr App::MakeWindow(const std::string& name, const WindowDesc& settings, void* native_wnd)
{
	return MakeSharedPtr<Window>(name, settings, native_wnd);
}

// 获取渲染目标的每秒帧数
/////////////////////////////////////////////////////////////////////////////////
uint32_t App::TotalNumFrames() const
{
	return m_nTotalNumFrames;
}

float App::FPS() const
{
	return m_fFPS;
}

float App::AppTime() const
{
	return m_fAppTime;
}

float App::FrameTime() const
{
	return m_fFrameTime;
}

uint32_t App::Update(uint32_t pass)
{
	if (0 == pass)
	{
		this->UpdateStats();
		this->DoUpdateOverlay();
	}

	return 0;
}

void App::UpdateStats()
{
	++m_nTotalNumFrames;

	// measure statistics
	m_fFrameTime = static_cast<float>(m_Timer.Elapsed());
	++n_nNumFrame;
	m_fAccumulate += m_fFrameTime;
	m_fAppTime += m_fFrameTime;

	// check if new second
	if (m_fAccumulate > 1)
	{
		// new second - not 100% precise
		m_fFPS = n_nNumFrame / m_fAccumulate;
		m_fAccumulate = 0;
		n_nNumFrame = 0;
	}

	m_Timer.ReStart();
}

void App::DoUpdateOverlay()
{
	Context::Instance()->DisPlay(m_fFrameTime);
}
