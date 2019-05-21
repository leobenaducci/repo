
#include "Render.h"

class RenderOGL : public IRender
{
public:
	virtual void OnWindowCreated(IWindow*) override;
	virtual void OnWindowDestroyed(IWindow*) override;
	virtual void PaintWindow(IWindow*) override;

	virtual void DrawLine(const Vector2& Start, const Vector2& End, const Vector4& Color) override;
	virtual void DrawRect(const Vector2& Pos, const Vector2& Size, const Image& Image, const Vector4& Color) override;
	virtual void DrawString(const Vector2& Pos, const Font& Font, wchar_t* Text) override;

	virtual void DrawLines(const std::vector<IRender::Vertex>& Vertices, bool bStrip = false) override;
	virtual void DrawPoly(const std::vector<IRender::Vertex>& Vertices) override;

	virtual void SetClipRect(const Vector2& Min, const Vector2& Max) override;
};
