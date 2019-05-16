#include "Vector.h"

class IWindow;
class Image {};
class Font {};

class IRender
{
public:
	virtual void OnWindowCreated(IWindow*) = 0;
	virtual void OnWindowDestroyed(IWindow*) = 0;	
	virtual void PaintWindow(IWindow*) = 0;

	virtual void DrawLine(const Vector2& Start, const Vector2& End, const Vector4& Color) = 0;
	virtual void DrawRect(const Vector2& Pos, const Vector2& Size, const Image& Image, const Vector4& Color) = 0;
	virtual void DrawString(const Vector2& Pos, const Font& Font, wchar_t* Text) = 0;

	virtual void SetClipRect(const Vector2& Min, const Vector2& Max) = 0;
};

IRender& GetRender();