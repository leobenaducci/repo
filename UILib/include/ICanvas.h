#pragma once

#include "Vector.h"
#include <vector>

class ICanvas
{
public:
	virtual bool IsPlatformWindow() const = 0;
	
	virtual void SetPosition(Vector2 NewPosition) = 0;
	virtual void SetSize(Vector2 NewSize) = 0;

	virtual Vector2 GetPosition() const = 0;
	virtual Vector2 GetSize() const = 0;

	virtual void UpdatePositionAndSize() = 0;
	virtual class IWindow* GetParentWindow() = 0;
};

