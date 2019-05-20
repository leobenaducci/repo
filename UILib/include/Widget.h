#pragma once

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
	void SetPivot(Vector2 NewPivot);

	const Vector2& GetCachedPosition() { return CachedPosition; }
	const Vector2& GetCachedSize() { return CachedSize; }

	void SetAnchors(unsigned int NewAnchors);

	void UpdatePositionAndSize();

	template<typename T>
	Widget* AddChild(ICanvas* Canvas = nullptr)
	{
		auto&& Child = std::make_unique<T>();
		Child->ParentCanvas = Canvas ? Canvas : GetCanvas();
		Child->Parent = this;
		Childs.push_back(std::move(Child));
		Childs.back().get()->Init();
		return Childs.back().get();
	}

	virtual ICanvas* GetCanvas() { return this; }

	virtual void Init() {}
	virtual void Render();

	virtual bool OnMousePressed(int x, int y, int btn);
	virtual bool OnMouseReleased(int x, int y, int btn);
	virtual bool OnMouseMoved(int OldX, int OldY, int NewX, int NewY);

	Vector4 Color = Vector4(1,1,1,1);

protected:
	ICanvas* ParentCanvas = nullptr;
	Widget* Parent = nullptr;

	Vector2 OffsetTopLeft = Vector2(0.f);
	Vector2 OffsetBottomRight = Vector2(0.f);
	Vector2 Position = Vector2(0.f);
	Vector2 Size = Vector2(0.f);
	Vector2 Pivot = Vector2(0.5f);

	Vector2 CachedPosition = Vector2(0.f);
	Vector2 CachedSize = Vector2(0.f);

	unsigned int Anchors = EAnchor::Left | EAnchor::Top;

	std::vector<std::unique_ptr<Widget>> Childs;

	friend class PlatformWindows;
};

