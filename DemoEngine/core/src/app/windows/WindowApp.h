#pragma once
#include <core/IApp.h>
#include <windows.h>

namespace CoreWorker
{
class WindowApp : public IApp 
{
public:
	WindowApp(const std::string& strName, const FWindowDesc& Settings, void* pNativeWnd);
	~WindowApp();

	HWND GetHWnd() const
	{
		return Hwnd_;
	}

	virtual void Create() override;
	virtual void Run() override;
    virtual void Close() override;
private:
	LRESULT MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    void UpdateDpiScale(float scale);

private:
	uint32_t 		WinStype_;
	HWND 			Hwnd_;
	WNDPROC 		DefaultWndProc_;
	bool            bExternalWnd_ = false;
};
}