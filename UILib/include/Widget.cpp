#include "Render.h"
#include "Widget.h"

#include "Vector.h"
#include <vector>

Vector2 Widget::GetPosition() const
{
	return CachedPosition + Canvas->GetPosition();
}

Vector2 Widget::GetSize() const
{
	return CachedSize;
}

const std::vector<Widget*>& Widget::GetChilds() const
{
	return Childs;
}

void Widget::SetPosition(Vector2 NewPosition)
{
	Parent->UpdatePositionAndSize();

	Position = NewPosition;

	OffsetTopLeft = Position - Size * Pivot;
	OffsetBottomRight = Position + Size * (Vector2(1.f) - Pivot) - Canvas->GetSize();
}

void Widget::SetSize(Vector2 NewSize)
{
	Parent->UpdatePositionAndSize();

	Size = NewSize;

	OffsetTopLeft = Position - Size * Pivot;
	OffsetBottomRight = Position + Size * (Vector2(1.f) - Pivot) - Canvas->GetSize();
}

void Widget::SetOffsets(Vector2 TopLeft, Vector2 BottomRight)
{
	Parent->UpdatePositionAndSize();

	OffsetTopLeft = TopLeft;
	OffsetBottomRight = BottomRight;

	Position = TopLeft * (Vector2(1.f) - Pivot) + BottomRight * Pivot;
	Size = BottomRight - TopLeft;
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
		CachedPosition.x() = Canvas->GetSize().x() + OffsetBottomRight.x();
		CachedSize.x() = Size.x();
	}
	else
	{
		CachedPosition.x() = (Canvas->GetPosition().x() + Canvas->GetSize().x()) * 0.5f + OffsetTopLeft.x();
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
		CachedPosition.y() = Canvas->GetSize().y() + OffsetBottomRight.y();
		CachedSize.y() = Size.y();
	}
	else
	{
		CachedPosition.y() = (Canvas->GetPosition().y() + Canvas->GetSize().y()) * 0.5f + OffsetTopLeft.y();
	}
}

void Widget::OnSizeChanged()
{
	UpdatePositionAndSize();

	for (auto it : Childs)
		it->OnSizeChanged();
}

void Widget::Render()
{
	GetRender().DrawRect(GetPosition(), GetSize(), Image(), Color);
}



