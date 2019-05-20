#pragma once

#include "Widget.h"
#include <vector>
#include <memory>

class WindowWidget : public Widget
{
public:
	WindowWidget();

	virtual void Init() override;
	
	virtual ICanvas* GetCanvas() override { return CanvasWidget; }

	virtual bool OnMousePressed(int x, int y, int btn) override;
	virtual bool OnMouseReleased(int x, int y, int btn) override;
	virtual bool OnMouseMoved(int OldX, int OldY, int NewX, int NewY) override;

protected:
	Widget* TitleBar;
	Widget* CanvasWidget;

};

