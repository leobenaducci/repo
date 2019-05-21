#pragma once

#include "Widget.h"
#include <vector>
#include <memory>

class WindowBarButton : public Widget
{
public:
	enum class EButtonType
	{
		Minimize,
		Maximize,
		Close,
	};

	EButtonType Type = EButtonType::Minimize;

	virtual void Render() override;
};

class WindowWidget : public Widget
{
public:
	WindowWidget();

	virtual void Init() override;
	
	virtual ICanvas* GetDefaultCanvas() override { return CanvasWidget; }

	virtual bool OnMousePressed(int x, int y, int btn) override;
	virtual bool OnMouseReleased(int x, int y, int btn) override;
	virtual bool OnMouseMoved(int OldX, int OldY, int NewX, int NewY) override;

	virtual void Render() override;

protected:
	Widget* TitleBar;
	WindowBarButton* MinimizeButton;
	WindowBarButton* MaximizeButton;
	WindowBarButton* CloseButton;

	Widget* CanvasWidget;

	bool bDragging = false;
};

