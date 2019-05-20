#include "WindowWidget.h"
#include "Render.h"

WindowWidget::WindowWidget()
{
}

void WindowWidget::Init()
{
	Widget::Init();

	SetSize(Vector2(200.f, 100.f));

	TitleBar = AddChild<Widget>(this);
	TitleBar->SetPivot(Vector2(0.f, 0.f));
	TitleBar->SetPosition(Vector2(0.f, 0.f));
	TitleBar->SetSize(Vector2(200.f, 20.f));
	TitleBar->SetAnchors(EAnchor::Top | EAnchor::Left | EAnchor::Right);
	TitleBar->Color = Vector4(1, 0, 0, 1);

	CanvasWidget = AddChild<Widget>(this);
	CanvasWidget->SetPivot(Vector2(0.f, 0.f));
	CanvasWidget->SetPosition(Vector2(0.f, 20));
	CanvasWidget->SetSize(Vector2(200.f, 180.f));
	CanvasWidget->SetAnchors(EAnchor::All);
	CanvasWidget->Color = Vector4(0, 1, 0, 1);
}
