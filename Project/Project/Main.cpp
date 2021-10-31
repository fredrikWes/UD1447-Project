#include "Application.h"
#include "DataTypes.h"

int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	Application app(hInstance);
	app.Run();

	return 0;
}