#pragma once

#include "Vector.h"
#include <vector>

class ICanvas
{
public:
	virtual Vector2 GetPosition() const = 0;
	virtual Vector2 GetSize() const = 0;
};

