#include "Window.h"

LRESULT CALLBACK Window::WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Window* wnd = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	if (msg == WM_NCCREATE)
	{
		LPCREATESTRUCT cs = reinterpret_cast<LPCREATESTRUCT>(lParam);
		wnd = reinterpret_cast<Window*>(cs->lpCreateParams);

		SetLastError(0);

		if (!SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(wnd)))
			if (GetLastError() != 0)
				return FALSE;
	}

	else
		return wnd->MessageHandler(hWnd, msg, wParam, lParam);

	if (wnd)
		return wnd->MessageHandler(hWnd, msg, wParam, lParam);

	else
		return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT Window::MessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CLOSE:
	{
		wantExit = true;
		break;
	}
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}