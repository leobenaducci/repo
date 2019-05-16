
#include <Windows.h>
#include "Platform.h"
#include "Widget.h"

int main(int argc, char** argv)
{
	WINDOW_CREATION_PARAMS Params;
	Params.Title = "Main";
	Params.X = 800;
	Params.Y = 600;
	Params.Width = 800;
	Params.Height = 600;
	IWindow* MainWnd = GetPlatform().NewWindow(Params);

	Widget* SubWidget = nullptr;

	SubWidget = MainWnd->GetCanvas()->AddChild<Widget>();
	SubWidget->SetPosition(Vector2(100, 100));
	SubWidget->SetSize(Vector2(200, 200));
	SubWidget->SetAnchors(EAnchor::Top | EAnchor::Left);
	SubWidget->Color = Vector4(1, 0, 0, 1);

	SubWidget = MainWnd->GetCanvas()->AddChild<Widget>();
	SubWidget->SetPosition(Vector2(100, 100));
	SubWidget->SetSize(Vector2(200, 200));
	SubWidget->SetAnchors(EAnchor::Bottom | EAnchor::Right);
	SubWidget->Color = Vector4(0, 0, 1, 1);

	//Params.Title = "Child";
	//Params.Parent = MainWnd;
	//GetPlatform().NewWindow(Params);

	while (true)
	{
		GetPlatform().Tick();

		if (GetPlatform().RequestingExit())
			break;
	}

	return 0;
}