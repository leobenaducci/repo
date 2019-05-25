#include "Render.h"
#include "Widget.h"

#include "Vector.h"
#include <vector>
#include <algorithm>

Widget::~Widget()
{
}

Vector2 Widget::GetPosition() const
{
	return CachedPosition + ParentCanvas->GetPosition();
}

Vector2 Widget::GetSize() const
{
	return CachedSize;
}

std::vector<Widget*> Widget::GetChilds() const
{
	std::vector<Widget*> Result;
	for (auto& It : Childs)
	{
		Result.push_back(It.get());
	}

	return std::move(Result);
}

void Widget::SetPosition(Vector2 NewPosition)
{
	Position = NewPosition;

	if (ParentCanvas)
	{
		ParentCanvas->UpdatePositionAndSize();

		OffsetTopLeft = Position - Size * Pivot;
		OffsetBottomRight = Position + Size * (Vector2(1.f) - Pivot) - ParentCanvas->GetSize();
	}
}

void Widget::SetSize(Vector2 NewSize)
{
	Size = Max(NewSize, Vector2(1.f, 1.f));

	if (ParentCanvas)
	{
		ParentCanvas->UpdatePositionAndSize();

		OffsetTopLeft = Position - Size * Pivot;
		OffsetBottomRight = Position + Size * (Vector2(1.f) - Pivot) - ParentCanvas->GetSize();
	}
}

void Widget::SetPivot(Vector2 NewPivot)
{
	Pivot = NewPivot;
}

void Widget::SetAnchors(unsigned int NewAnchors)
{
	Anchors = NewAnchors;
}

void Widget::UpdatePositionAndSize()
{
	if (ParentCanvas)
	{
		ParentCanvas->UpdatePositionAndSize();
	}

#if 1
	auto Anchoring = [this](int Anchor1, int Anchor2, float& (Vector2::*Axis)(void))
	{
		if (Anchors & Anchor1 && Anchors & Anchor2)
		{
			(CachedPosition.*Axis)() = (OffsetTopLeft.*Axis)();
			(CachedSize.*Axis)() = ((ParentCanvas->GetSize().*Axis)() + (OffsetBottomRight.*Axis)()) - (OffsetTopLeft.*Axis)();
		}
		else if (Anchors & Anchor1)
		{
			(CachedPosition.*Axis)() = (OffsetTopLeft.*Axis)();
			(CachedSize.*Axis)() = (Size.*Axis)();
		}
		else if (Anchors & Anchor2)
		{
			(CachedPosition.*Axis)() = (ParentCanvas->GetSize().*Axis)() + (OffsetTopLeft.*Axis)();
			(CachedSize.*Axis)() = (Size.*Axis)();
		}
		else
		{
			(CachedPosition.*Axis)() = ((ParentCanvas->GetPosition().*Axis)() - (Size.*Axis)() * (Pivot.*Axis)()) + (ParentCanvas->GetSize().*Axis)() * 0.5f;
			(CachedSize.*Axis)() = (Size.*Axis)();
		}

	};

	Anchoring(EAnchor::Left, EAnchor::Right, &Vector2::x);
	Anchoring(EAnchor::Top, EAnchor::Bottom, &Vector2::y);

#else

	if (Anchors & EAnchor::Left && Anchors & EAnchor::Right)
	{
		CachedPosition.x() = OffsetTopLeft.x();
		CachedSize.x() = (ParentCanvas->GetSize().x() + OffsetBottomRight.x()) - OffsetTopLeft.x();
	}
	else if (Anchors & EAnchor::Left)
	{
		CachedPosition.x() = OffsetTopLeft.x();
		CachedSize.x() = Size.x();
	}
	else if (Anchors & EAnchor::Right)
	{
		CachedPosition.x() = ParentCanvas->GetSize().x() + OffsetTopLeft.x();
		CachedSize.x() = Size.x();
	}
	else
	{
		CachedPosition.x() = (ParentCanvas->GetPosition().x() - Size.x() * Pivot.x()) + ParentCanvas->GetSize().x() * 0.5f;
		CachedSize.x() = Size.x();
	}

	if (Anchors & EAnchor::Top && Anchors & EAnchor::Bottom)
	{
		CachedPosition.y() = OffsetTopLeft.y();
		CachedSize.y() = (ParentCanvas->GetSize().y() + OffsetBottomRight.y()) - OffsetTopLeft.y();
	}
	else if (Anchors & EAnchor::Top)
	{
		CachedPosition.y() = OffsetTopLeft.y();
		CachedSize.y() = Size.y();
	}
	else if (Anchors & EAnchor::Bottom)
	{
		CachedPosition.y() = ParentCanvas->GetSize().y() + OffsetTopLeft.y();
		CachedSize.y() = Size.y();
	}
	else
	{
		CachedPosition.y() = (ParentCanvas->GetPosition().y() - Size.y() * Pivot.y() + ParentCanvas->GetSize().y() * 0.5f);
		CachedSize.y() = Size.y();
	}
#endif
}

void Widget::RemoveFromParent()
{
	if (Parent == nullptr)
		return;

	auto it = std::find_if(Parent->Childs.begin(), Parent->Childs.end(), [this](const std::unique_ptr<Widget>& A) { return A.get() == this; } );
	if (it != Parent->Childs.end())
	{
		it->release();
		Parent->Childs.erase(it);
	}
}

void Widget::Render()
{
	UpdatePositionAndSize();
	GetRender().DrawRect(GetPosition(), GetSize(), Image(), Color);

	for (auto it : GetChilds())
	{
		it->Render();
	}
}

bool Widget::OnMousePressed(int x, int y, int btn)
{
	for (int i = Childs.size()-1; i >= 0; i--)
	{
		auto It = Childs[i].get();

		if (x >= It->GetPosition().x() && x <= It->GetPosition().x() + It->GetSize().x() &&
			y >= It->GetPosition().y() && y <= It->GetPosition().y() + It->GetSize().y())
		{
			if (It->OnMousePressed(x, y, btn))
				return true;
		}
	}
	
	return false;
}

bool Widget::OnMouseReleased(int x, int y, int btn)
{
	for (auto It : GetChilds())
	{
		It->OnMouseReleased(x, y, btn);
	}

	return false;
}

bool Widget::OnMouseMoved(int OldX, int OldY, int NewX, int NewY)
{
	for (auto It : GetChilds())
	{
		It->OnMouseMoved(OldX, OldY, NewX, NewY);
	}

	return false;
}

