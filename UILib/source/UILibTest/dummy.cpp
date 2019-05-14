
#include <Windows.h>
#include "Platform.h"

int main(int argc, char** argv)
{
	UI_WINDOW_CREATION_PARAMS Params;
	Params.Title = "Main";
	Params.X = 800;
	Params.Y = 600;
	Params.Width = 800;
	Params.Height = 600;
	IWindow* MainWnd = GetPlatform().NewWindow(Params);

	Params.Title = "Child";
	Params.Parent = MainWnd;
	GetPlatform().NewWindow(Params);

	while (true)
	{
		GetPlatform().Tick();

		if (GetPlatform().RequestingExit())
			break;
	}

	return 0;
}