#include "Render.h"
#include "Widget.h"

#include "Vector.h"
#include <vector>

Widget::~Widget()
{
}

Vector2 Widget::GetPosition() const
{
	return CachedPosition + Canvas->GetPosition();
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

	if (Parent)
	{
		Parent->UpdatePositionAndSize();

		OffsetTopLeft = Position - Size * Pivot;
		OffsetBottomRight = Position + Size * (Vector2(1.f) - Pivot) - Canvas->GetSize();
	}
}

void Widget::SetSize(Vector2 NewSize)
{
	Size = NewSize;

	if (Parent)
	{
		Parent->UpdatePositionAndSize();

		OffsetTopLeft = Position - Size * Pivot;
		OffsetBottomRight = Position + Size * (Vector2(1.f) - Pivot) - Canvas->GetSize();
	}
}

void Widget::SetOffsets(Vector2 TopLeft, Vector2 BottomRight)
{
	Parent->UpdatePositionAndSize();
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
	if (Anchors & EAnchor::Left && Anchors & EAnchor::Right)
	{
		CachedPosition.x() = OffsetTopLeft.x();
		CachedSize.x() = (Canvas->GetSize().x() + OffsetBottomRight.x()) - OffsetTopLeft.x();
	}
	else if (Anchors & EAnchor::Left)
	{
		CachedPosition.x() = OffsetTopLeft.x();
		CachedSize.x() = Size.x();
	}
	else if (Anchors & EAnchor::Right)
	{
		CachedPosition.x() = Canvas->GetSize().x() + OffsetTopLeft.x();
		CachedSize.x() = Size.x();
	}
	else
	{
		CachedPosition.x() = (Canvas->GetPosition().x() - Size.x() * Pivot.x()) + Canvas->GetSize().x() * 0.5f;
		CachedSize.x() = Size.x();
	}

	if (Anchors & EAnchor::Top && Anchors & EAnchor::Bottom)
	{
		CachedPosition.y() = OffsetTopLeft.y();
		CachedSize.y() = (Canvas->GetSize().y() + OffsetBottomRight.y()) - OffsetTopLeft.y();
	}
	else if (Anchors & EAnchor::Top)
	{
		CachedPosition.y() = OffsetTopLeft.y();
		CachedSize.y() = Size.y();
	}
	else if (Anchors & EAnchor::Bottom)
	{
		CachedPosition.y() = Canvas->GetSize().y() + OffsetTopLeft.y();
		CachedSize.y() = Size.y();
	}
	else
	{
		CachedPosition.y() = (Canvas->GetPosition().y() - Size.y() * Pivot.y() + Canvas->GetSize().y() * 0.5f);
		CachedSize.y() = Size.y();
	}
}

void Widget::OnSizeChanged()
{
	for (auto& it : Childs)
		it->OnSizeChanged();
}

void Widget::Render()
{
	GetRender().DrawRect(GetPosition(), GetSize(), Image(), Color);
}



