// 2018年9月10日 窗口创建改为类
#ifndef _STX_WINDOW_H_
#define _STX_WINDOW_H_
#pragma once

#include "../Container/macro.h"
#include "../Core/predefine.h"
#include "../Core/Context.h"

#if defined STX_PLATFORM_WINDOWS_DESKTOP
#include <windows.h>
#elif defined STX_PLATFORM_LINUX
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#endif

#include <boost/noncopyable.hpp>
#include <boost/signals2.hpp>
class Window : boost::noncopyable
{
public:
	Window(std::string const & name, const WindowDesc& settings, void* native_wnd);

	~Window();

#if defined STX_PLATFORM_WINDOWS_DESKTOP
	HWND GetHWnd() const
	{
		return m_Hwnd;
	}
#endif

	typedef boost::signals2::signal<void(const Window& wnd, bool active)> ActiveEvent;
	typedef boost::signals2::signal<void(const Window& wnd, wchar_t ch)> CharEvent;
	typedef boost::signals2::signal<void(const Window& wnd, bool active)> SizeEvent;
#if defined STX_PLATFORM_WINDOWS_DESKTOP
	typedef boost::signals2::signal<void(const Window& wnd, HRAWINPUT ri)> RawInputEvent;
	typedef boost::signals2::signal<void(const Window& wnd, const int2& pt, uint32_t id)> PointerDownEvent;
	typedef boost::signals2::signal<void(const Window& wnd, const int2& pt, uint32_t id)> PointerUpEvent;
	typedef boost::signals2::signal<void(const Window& wnd, const int2& pt, uint32_t id, bool down)> PointerUpdateEvent;
	typedef boost::signals2::signal<void(const Window& wnd, const int2& pt, uint32_t id, int32_t wheel_delta)> PointerWheelEvent;
	typedef boost::signals2::signal<void(const Window& wnd)> CloseEvent;
#endif

	int32_t Left() const{return m_nTop;}
	int32_t Top() const{return m_nTop;}
	uint32_t Width() const{return m_nWidth;}
	uint32_t Height() const{return m_Height;}
	bool Active() const{return m_bActive;}
	void Active(bool active){m_bActive = active;}
	bool Ready() const{return m_bReady;}
	void Ready(bool ready){m_bReady = ready;}
	bool Closed() const{return m_bClosed;}
	void Closed(bool closed){m_bClosed = closed;}
	bool Windows() { return m_bKeepScreenOn; }
	void Windows(bool bWin) { m_bKeepScreenOn = bWin; }

	ActiveEvent& OnActive() { return m_sgActiveEvent; }
	CharEvent& OnChar() { return m_sgCharEvent; }
	SizeEvent& OnSize() { return m_sgSizeEvent; }
#if defined STX_PLATFORM_WINDOWS_DESKTOP
	RawInputEvent& OnRawInput() { return m_sgRawInputEvent; }
	CloseEvent& OnClose() { return m_sgCloseEvent; }
	PointerDownEvent& OnPointerDown() { return m_sgPointerDownEvent; }
	PointerUpEvent& OnPointerUp() { return m_sgPointerUpEvent; }
	PointerUpdateEvent& OnPointerUpdate() { return m_sgPointerUpdateEvent; }
	PointerWheelEvent& OnPointerWheel() { return m_sgPointerWheelEvent; }
#endif
private:
#if defined STX_PLATFORM_WINDOWS_DESKTOP
	LRESULT MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#elif defined STX_PLATFORM_LINUX
	void MsgProc(XEvent const & event);
#endif

	void UpdateDpiScale(float scale);
private:
	int32_t m_nLeft;
	int32_t m_nTop;
	uint32_t m_nWidth;
	uint32_t m_Height;

	bool m_bActive;
	bool m_bReady;
	bool m_bClosed;
	bool m_bKeepScreenOn;

	float m_fDpiScale;

	ActiveEvent m_sgActiveEvent;
	CharEvent m_sgCharEvent;
	SizeEvent m_sgSizeEvent;
#if defined STX_PLATFORM_WINDOWS
	bool m_bHide;
	bool m_bExternalWnd;
	#if STX_PLATFORM_WIN64
		std::wstring m_swcName;
	#else
		std::string m_swcName;
	#endif
#endif

#if defined STX_PLATFORM_WINDOWS_DESKTOP
	RawInputEvent m_sgRawInputEvent;
	uint32_t m_WinStype;
	HWND m_Hwnd;
	WNDPROC m_DefaultWndProc;

	PointerDownEvent m_sgPointerDownEvent;
	PointerUpEvent m_sgPointerUpEvent;
	PointerUpdateEvent m_sgPointerUpdateEvent;
	PointerWheelEvent m_sgPointerWheelEvent;
	CloseEvent m_sgCloseEvent;
#endif
};
#endif//_STX_WINDOW_H_