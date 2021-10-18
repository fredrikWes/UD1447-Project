#pragma once
#include "Window.h"

class WindowCreator
{
	bool InitializeWindow(UINT width, UINT height, LPCWSTR title, HINSTANCE instance, Window& window);
};