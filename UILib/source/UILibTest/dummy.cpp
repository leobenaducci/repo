
#include <Windows.h>
#include "Platform.h"
#include "Widget.h"
#include "WindowWidget.h"

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

	Widget* MainWidget = SubWidget = MainWnd->GetCanvas()->AddChild<WindowWidget>();
	SubWidget->SetAnchors(EAnchor::Top | EAnchor::Left);
	SubWidget->SetPivot(Vector2(0.0f, 0.0f));
	SubWidget->SetPosition(Vector2(300, 200));
	SubWidget->SetSize(Vector2(200, 300));

	MainWidget = SubWidget = MainWnd->GetCanvas()->AddChild<WindowWidget>();
	SubWidget->SetAnchors(EAnchor::Top | EAnchor::Left);
	SubWidget->SetPivot(Vector2(0.0f, 0.0f));
	SubWidget->SetPosition(Vector2(100, 100));
	SubWidget->SetSize(Vector2(400, 300));

	SubWidget = SubWidget->AddChild<Widget>();
	SubWidget->SetAnchors(EAnchor::All);
	SubWidget->SetPivot(Vector2(0.0f, 0.0f));
	SubWidget->SetPosition(Vector2(50, 50));
	SubWidget->SetSize(Vector2(50, 50));
	SubWidget->Color = Vector4(1, 0, 1, 1);
	
	SubWidget = SubWidget->AddChild<Widget>();
	SubWidget->SetPosition(Vector2(100, 100));
	SubWidget->SetPivot(Vector2(0.f, 0.f));
	SubWidget->SetSize(Vector2(200, 100));
	SubWidget->SetAnchors(EAnchor::All);
	SubWidget->Color = Vector4(0, 0, 1, 1);
	
	Params.Title = "Child";
	Params.Parent = MainWnd;
	Params.Width = 400;
	Params.Height = 400;
	IWindow* ChildWnd = GetPlatform().NewWindow(Params);
	
	SubWidget = ChildWnd->GetCanvas()->AddChild<Widget>();
	SubWidget->SetPosition(Vector2(-100, 100));
	SubWidget->SetPivot(Vector2(1.f, 0.f));
	SubWidget->SetSize(Vector2(100, 100));
	SubWidget->SetAnchors(EAnchor::Top | EAnchor::Right);
//	SubWidget->SetAnchors(EAnchor::All);
	SubWidget->Color = Vector4(1, 1, 0, 1);
	
	SubWidget = ChildWnd->GetCanvas()->AddChild<Widget>();
	SubWidget->SetPosition(Vector2(100, -100));
	SubWidget->SetPivot(Vector2(0.f, 1.f));
	SubWidget->SetSize(Vector2(50, 50));
	SubWidget->SetAnchors(EAnchor::Bottom | EAnchor::Left);
//	SubWidget->SetAnchors(EAnchor::All);
	SubWidget->Color = Vector4(0, 1, 1, 1);

	while (true)
	{
		GetPlatform().Tick();
		if (GetPlatform().RequestingExit())
			break;
	}

	return 0;
}