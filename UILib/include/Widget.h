#include "ICanvas.h"
#include "Vector.h"
#include <vector>
#include <memory>

namespace EAnchor
{
	enum Type : unsigned int
	{
		Center = 0x0,
		Left = 0x1,
		Right = 0x2,
		Top = 0x4,
		Bottom = 0x8,

		All = Left | Right | Top | Bottom,
	};
}

class Widget : public ICanvas
{
public:
	virtual ~Widget();

	virtual Vector2 GetPosition() const override;
	virtual Vector2 GetSize() const override;
	std::vector<Widget*> GetChilds() const;

	void SetPosition(Vector2 NewPosition);
	void SetSize(Vector2 NewSize);
	void SetOffsets(Vector2 TopLeft, Vector2 BottomRight);
	void SetPivot(Vector2 NewPivot);

	void SetAnchors(unsigned int NewAnchors);

	void UpdatePositionAndSize();

	template<typename T>
	Widget* AddChild()
	{
		auto&& Child = std::make_unique<T>();
		Child->Canvas = this;
		Child->Parent = this;
		Childs.push_back(std::move(Child));
		return Childs.back().get();
	}

	virtual void OnSizeChanged();
	virtual void Render();

	Vector4 Color = Vector4(1,1,1,1);

	ICanvas* Canvas = nullptr;
	Widget* Parent = nullptr;

protected:
	Vector2 OffsetTopLeft = Vector2(0.f);
	Vector2 OffsetBottomRight = Vector2(0.f);
	Vector2 Position = Vector2(0.f);
	Vector2 Size = Vector2(0.f);
	Vector2 Pivot = Vector2(0.5f);

	Vector2 CachedPosition = Vector2(0.f);
	Vector2 CachedSize = Vector2(0.f);

	unsigned int Anchors = EAnchor::Left | EAnchor::Top;

	std::vector<std::unique_ptr<Widget>> Childs;
};

