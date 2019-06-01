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

	virtual bool OnMousePressed(int x, int y, int btn);
	virtual bool OnMouseReleased(int x, int y, int btn);
};

class WindowWidget : public Widget
{
public:
	WindowWidget();

	virtual void Init() override;
	
	virtual void SetPivot(Vector2 NewPivot) override {}
	virtual void SetAnchors(unsigned int NewAnchors) override {}

	virtual ICanvas* GetDefaultCanvas() override { return CanvasWidget; }

	virtual bool OnMousePressed(int x, int y, int btn) override;
	virtual bool OnMouseReleased(int x, int y, int btn) override;
	virtual bool OnMouseMoved(int OldX, int OldY, int NewX, int NewY) override;

	virtual void Render() override;

	std::string Name;
	bool bCanCreatePlatformWindow = false;

protected:
	Widget* TitleBar;
	WindowBarButton* MinimizeButton;
	WindowBarButton* MaximizeButton;
	WindowBarButton* CloseButton;

	Widget* CanvasWidget;

	bool bDragging = false;
	bool bScaleVerticalFromTop = false;
	bool bScaleVerticalFromBottom = false;
	bool bScaleHorizontalFromLeft = false;
	bool bScaleHorizontalFromRight = false;
};

