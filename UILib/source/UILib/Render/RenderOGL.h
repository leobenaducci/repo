
#include "Render.h"
#include "RenderContext.h"

class RenderOGL : public IRender
{
public:
	virtual void OnWindowCreated(IWindow*) override;
	virtual void OnWindowDestroyed(IWindow*) override;
	virtual void PaintWindow(IWindow*) override;
};

class RenderContextOGL : public IRenderContext
{
public:
	void DrawLine(const struct Vector2& Start, const struct Vector2& End, const struct Vector4& Color) {}
	void DrawRect(const struct Vector2& Pos, const struct Vector2& Size, const Image& Image, const struct Vector4& Color) {}
	void DrawString(const struct Vector2& Pos, const struct Font& Font, wchar_t* Text) {}

	void SetClipRect(const struct Vector2& Min, const struct Vector2& Max) {}
};

IRenderContext& GetRenderContext()
{
	static RenderContextOGL RenderContext;
	return RenderContext;
}
