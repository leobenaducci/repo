#include "ICanvas.h"
#include <memory>

struct WINDOW_CREATION_PARAMS
{
	char* Title = "Window";
	unsigned int X = 0;
	unsigned int Y = 0;
	unsigned int Width = 0;
	unsigned int Height = 0;
	class IWindow* Parent = nullptr;
	bool bStartDragging = false;
};

class IWindow : public ICanvas
{
public:
	class Widget* GetCanvas() const { return Canvas.get(); }
	virtual Vector2 PositionToAbsolute(Vector2 RelativePosition) const = 0;
	
	virtual void Move(Vector2 Offset) = 0;
	void SetCapturedWidget(Widget* widget) { CapturedWidget = widget; }
	void ReleaseCapturedWidget() { CapturedWidget = nullptr; }

	template<typename T>
	T* AddChild()
	{
		return Canvas->AddChild<T>(this);
	}

protected:
	class std::unique_ptr<Widget> Canvas;
	Widget* CapturedWidget = nullptr;
};

class IPlatform
{
public:
	virtual bool SupportsMultipleWindows() const = 0;
	virtual bool RequestingExit() const = 0;

	virtual IWindow* NewWindow(struct WINDOW_CREATION_PARAMS&) = 0;
	virtual bool DestroyWindow(IWindow*) = 0;
	
	virtual void Tick() = 0;
};

IPlatform& GetPlatform();
