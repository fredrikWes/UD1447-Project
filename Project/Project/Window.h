#pragma once
#include <Windows.h>
#include <iostream>

class Window
{
	friend class WindowCreator;
private:
	static bool wantExit;

	static UINT width, height;
	static HWND hWnd;

	static LRESULT MessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
public:
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	static bool Initialize(UINT width, UINT height, LPCWSTR title, HINSTANCE instance);
	static void ShutDown() { DestroyWindow(hWnd); }

	static UINT ClientWidth() { return width; }
	static UINT ClientHeight() { return height; }

	static bool Exit() { return wantExit; }
	static HWND GetHWND() { return hWnd; }
};

inline bool Window::wantExit = false;
inline UINT Window::width;
inline UINT Window::height;
inline HWND Window::hWnd;

struct WindowCreator
{
	void Initialize(Window& window, UINT width, UINT height, LPCWSTR title, HINSTANCE instance)
	{
		const wchar_t* className = L"Window Class";

		WNDCLASS wc = { 0 };
		wc.lpfnWndProc = window.WindowProc;
		wc.hInstance = instance;
		wc.lpszClassName = className;

		RegisterClass(&wc);

		window.hWnd = CreateWindowEx(0, className, title,
			WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
			CW_USEDEFAULT, CW_USEDEFAULT, width, height,
			nullptr, nullptr, instance, this);

		RAWINPUTDEVICE rid = {};
		rid.usUsagePage = 0x01;
		rid.usUsage = 0x02;
		rid.dwFlags = 0;
		rid.hwndTarget = nullptr;
		RegisterRawInputDevices(&rid, 1, sizeof(rid));

#ifdef _DEBUG
		AllocConsole();
		(void)freopen("conout$", "w", stdout);
#endif

		ShowWindow(window.hWnd, SW_SHOWDEFAULT);

		RECT clientRect;
		GetClientRect(window.hWnd, &clientRect);
		window.width = clientRect.right;
		window.height = clientRect.bottom;
	}
};