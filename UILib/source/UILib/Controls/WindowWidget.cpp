#include "Platform.h"
#include "WindowWidget.h"
#include "Render.h"

WindowWidget::WindowWidget()
{
}

void WindowWidget::Init()
{
	Widget::Init();

	SetSize(Vector2(202.f, 202.f));
	Color = Vector4(0.25f, 0.25f, 0.25f, 1.f);

	TitleBar = AddChild<Widget>(this);
	TitleBar->SetPivot(Vector2(0.f, 0.f));
	TitleBar->SetPosition(Vector2(1.f, 1.f));
	TitleBar->SetSize(Vector2(200.f, 30.f));
	TitleBar->SetAnchors(EAnchor::Top | EAnchor::Left | EAnchor::Right);
	TitleBar->Color = Vector4(0.5f, 0.5f, 0.8f, 1);

	MinimizeButton = TitleBar->AddChild<WindowBarButton>();
	MinimizeButton->Type = WindowBarButton::EButtonType::Minimize;
	MinimizeButton->SetPivot(Vector2(0.f, 0.f));
	MinimizeButton->SetPosition(Vector2(-120.f, 0.f));
	MinimizeButton->SetSize(Vector2(30.f, 30.f));
	MinimizeButton->SetAnchors(EAnchor::Top | EAnchor::Right);
	MinimizeButton->Color = Vector4(0, 0.75f, 0, 1);

	MaximizeButton = TitleBar->AddChild<WindowBarButton>();
	MaximizeButton->Type = WindowBarButton::EButtonType::Maximize;
	MaximizeButton->SetPivot(Vector2(0.f, 0.f));
	MaximizeButton->SetPosition(Vector2(-80.f, 0.f));
	MaximizeButton->SetSize(Vector2(30.f, 30.f));
	MaximizeButton->SetAnchors(EAnchor::Top | EAnchor::Right);
	MaximizeButton->Color = Vector4(0.75f, 0.75f, 0, 1);

	CloseButton = TitleBar->AddChild<WindowBarButton>();
	CloseButton->Type = WindowBarButton::EButtonType::Close;
	CloseButton->SetPivot(Vector2(0.f, 0.f));
	CloseButton->SetPosition(Vector2(-40.f, 0.f));
	CloseButton->SetSize(Vector2(30.f, 30.f));
	CloseButton->SetAnchors(EAnchor::Top | EAnchor::Right);
	CloseButton->Color = Vector4(0.75f, 0, 0, 1);

	CanvasWidget = AddChild<Widget>(this);
	CanvasWidget->SetPivot(Vector2(0.f, 0.f));
	CanvasWidget->SetPosition(Vector2(1.f, 31.f));
	CanvasWidget->SetSize(Vector2(200.f, 170.f));
	CanvasWidget->SetAnchors(EAnchor::All);
	CanvasWidget->Color = Vector4(0.8f, 0.8f, 0.8f, 1);
}

bool WindowWidget::OnMousePressed(int x, int y, int btn) 
{
	if (x >= TitleBar->GetCachedPosition().x() && x <= TitleBar->GetCachedPosition().x() + TitleBar->GetCachedSize().x() &&
		y >= TitleBar->GetCachedPosition().y() && y <= TitleBar->GetCachedPosition().y() + TitleBar->GetCachedSize().y())
	{
		//RemoveFromParent();
		//
		//WINDOW_CREATION_PARAMS Params;
		//Params.Title = "Main";
		//Params.X = GetPosition().x();
		//Params.Y = GetPosition().y();
		//Params.Width = GetSize().x();
		//Params.Height = GetSize().y();
		//IWindow* NewWnd = GetPlatform().NewWindow(Params);
		//
		//NewWnd->GetCanvas()->AddExistingChild(this);
		//
		//SetPivot(Vector2(0.f));
		//SetPosition(Vector2(1.f, -31.f));
		//SetSize(GetSize());
		//SetAnchors(EAnchor::All);

		bDragging = true;
	}
	return false;
}

bool WindowWidget::OnMouseReleased(int x, int y, int btn)
{
	bDragging = false;

	return false;
}

bool WindowWidget::OnMouseMoved(int OldX, int OldY, int NewX, int NewY)
{
	if (bDragging)
	{
		SetPosition(Position + Vector2((float)NewX - OldX, (float)NewY - OldY));
	}

	return false;
}

void WindowWidget::Render()
{
	UpdatePositionAndSize();

	IRender::Vertex v[5]
	{
		IRender::Vertex{Vector3(GetPosition() + GetSize() * Vector2(0.f, 0.f) + Vector2(-10.f, -10.f)), Vector4(0.f), Vector2(0.f)},
		IRender::Vertex{Vector3(GetPosition() + GetSize() * Vector2(1.f, 0.f) + Vector2(10.f, -10.f)), Vector4(0.f), Vector2(0.f)},
		IRender::Vertex{Vector3(GetPosition() + GetSize() * Vector2(1.f, 1.f) + Vector2(10.f, 10.f)), Vector4(0.f), Vector2(0.f)},
		IRender::Vertex{Vector3(GetPosition() + GetSize() * Vector2(0.f, 1.f) + Vector2(-10.f, 10.f)), Vector4(0.f), Vector2(0.f)},
		IRender::Vertex{Vector3(GetPosition() + GetSize() * 0.5f), Vector4(0.f, 0.f, 0.f, 0.75f), Vector2(0.f)},
	};

	std::vector<IRender::Vertex> Poly;
	Poly.push_back(v[0]);
	Poly.push_back(v[1]);
	Poly.push_back(v[4]);

	Poly.push_back(v[1]);
	Poly.push_back(v[2]);
	Poly.push_back(v[4]);

	Poly.push_back(v[2]);
	Poly.push_back(v[3]);
	Poly.push_back(v[4]);

	Poly.push_back(v[3]);
	Poly.push_back(v[0]);
	Poly.push_back(v[4]);

	GetRender().DrawPoly(Poly);

	Widget::Render();
}


void WindowBarButton::Render()
{
	UpdatePositionAndSize();

	Vector2 Min = (GetPosition().Floor() + GetSize().Ceil() * 0.3f);
	Vector2 Max = (GetPosition().Floor() + GetSize().Ceil() * 0.6f);

	switch(Type)
	{
	case EButtonType::Minimize:
		GetRender().DrawLine(Vector2(Min.x(), (Min.y() + Max.y()) * 0.5f), Vector2(Max.x(), (Min.y() + Max.y()) * 0.5f), Vector4(0.25, 0.25, 0.25, 1.f));
		break;
	case EButtonType::Maximize:
	{
		std::vector<IRender::Vertex> v;
		v.push_back(IRender::Vertex{ Vector3(Min.x(), Min.y()), Vector4(0.25, 0.25, 0.25, 1.f), Vector2() });
		v.push_back(IRender::Vertex{ Vector3(Max.x(), Min.y()), Vector4(0.25, 0.25, 0.25, 1.f), Vector2() });
		v.push_back(IRender::Vertex{ Vector3(Max.x(), Max.y()), Vector4(0.25, 0.25, 0.25, 1.f), Vector2() });
		v.push_back(IRender::Vertex{ Vector3(Min.x(), Max.y()), Vector4(0.25, 0.25, 0.25, 1.f), Vector2() });
		v.push_back(IRender::Vertex{ Vector3(Min.x(), Min.y()), Vector4(0.25, 0.25, 0.25, 1.f), Vector2() });

		GetRender().DrawLines(v, true);
		break;
	}
	case EButtonType::Close:
		GetRender().DrawLine(Vector2(Min.x(), Min.y()), Vector2(Max.x(), Max.y()), Vector4(0.25, 0.25, 0.25, 1.f));
		GetRender().DrawLine(Vector2(Min.x(), Max.y()), Vector2(Max.x(), Min.y()), Vector4(0.25, 0.25, 0.25, 1.f));
		break;
	};
}