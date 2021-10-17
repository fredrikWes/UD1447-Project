#pragma once
#include <Windows.h>

class Window
{
private:
	static bool wantExit;

	static UINT width, height;
	static HWND hWnd;

	static LRESULT MessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
public:
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	static void Initialize(UINT width, UINT height, LPCWSTR title, HINSTANCE instance);
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