
struct Image 
{
};

struct Font
{
};

class IRenderContext
{
public:

	virtual void DrawLine(const struct Vector2& Start, const struct Vector2& End, const struct Vector4& Color) = 0;
	virtual void DrawRect(const struct Vector2& Pos, const struct Vector2& Size, const Image& Image, const struct Vector4& Color) = 0;
	virtual void DrawString(const struct Vector2& Pos, const struct Font& Font, wchar_t* Text) = 0;

	virtual void SetClipRect(const struct Vector2& Min, const struct Vector2& Max) = 0;
};

IRenderContext& GetRenderContext();