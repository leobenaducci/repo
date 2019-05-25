#pragma once

#include "Vector.h"
#include <vector>

class ICanvas
{
public:
	virtual bool IsPlatformWindow() const = 0;
	virtual Vector2 GetPosition() const = 0;
	virtual Vector2 GetSize() const = 0;

	virtual void UpdatePositionAndSize() = 0;
};

