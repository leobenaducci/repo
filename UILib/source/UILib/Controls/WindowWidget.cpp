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

bool WindowWidget::OnMousePressed(int x, int y, int btn) 
{
	if (x >= TitleBar->GetCachedPosition().x() && x <= TitleBar->GetCachedPosition().x() + TitleBar->GetCachedSize().x() &&
		y >= TitleBar->GetCachedPosition().y() && y <= TitleBar->GetCachedPosition().y() + TitleBar->GetCachedSize().y())
	{
		TitleBar->Color = Vector4(0.95f, 0, 0, 1);
	}
	return false;
}

bool WindowWidget::OnMouseReleased(int x, int y, int btn)
{
	TitleBar->Color = Vector4(1, 0, 0, 1);
	return false;
}

bool WindowWidget::OnMouseMoved(int OldX, int OldY, int NewX, int NewY)
{
	if (TitleBar->Color.x() == 0.95f)
	{
		SetPosition(Position + Vector2(NewX - OldX, NewY - OldY));
	}

	return false;
}
