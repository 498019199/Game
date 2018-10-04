//2018年9月13日 流程控制
#ifndef STX_APP_H
#define STX_APP_H
#pragma once
#include "../Core/predefine.h"
#include "../Math/Math.h"
#include "../Util/Timer.h"
class App
{
public:
	 explicit App(const std::string& name, void* native_wnd);

	~App();

	void Create();
	void Destroy();

	void Run();
	void Quit();

	WindowPtr MakeWindow(const std::string& name, const WindowDesc& settings);
	WindowPtr MakeWindow(const std::string& name, const WindowDesc& settings, void* native_wnd);
	WindowPtr GetMainWin() { return m_MainWinPtr; }

	std::string GetName() { return m_strAppName; }
	uint32_t TotalNumFrames() const;
	float FPS() const;
	float AppTime() const;
	float FrameTime() const;
protected:
	void LookAt(float3 const & eye, float3 const & lookAt);
	void LookAt(float3 const & eye, float3 const & lookAt, float3 const & up);
	void Proj(float nearPlane, float farPlane);

protected:
	uint32_t Update(uint32_t pass);

	void UpdateStats();

	virtual void DoUpdateOverlay() = 0;
	virtual uint32_t DoUpdate(uint32_t pass) = 0;
private:
	virtual void OnCreate(){}
	virtual void OnDestroy(){}
private:
	std::string m_strAppName;
	WindowPtr m_MainWinPtr;

	// 时间，帧数管理
	Timer m_Timer;
	float m_fAppTime;
	float m_fFrameTime;
	uint32_t m_nTotalNumFrames;
	uint32_t n_nNumFrame;
	float m_fAccumulate;
	float m_fFPS;
};
#endif// STX_APP_H
