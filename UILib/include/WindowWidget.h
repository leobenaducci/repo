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

protected:
	Widget* TitleBar;
	Widget* CanvasWidget;

};

