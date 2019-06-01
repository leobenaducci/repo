
#include <Windows.h>
#include "Platform.h"

class PlatformWindows : public IPlatform
{
public:
	virtual bool SupportsMultipleWindows() const override;
	virtual bool RequestingExit() const override;

	PlatformWindows();
	virtual ~PlatformWindows();

	virtual IWindow* NewWindow(struct WINDOW_CREATION_PARAMS&) override;
	virtual bool DestroyWindow(IWindow*) override;
	
	virtual void Tick() override;

private:
	static LRESULT WndProc(HWND, UINT, WPARAM, LPARAM);
};

class WindowWindows : public IWindow
{
public:
	void* Handle;
	IWindow* Parent;

	bool bIgnoreMouseMoveEvent = false;
	bool bDragging = false;
	int LastMouseX = -1, LastMouseY = -1;
	int LastPositionX = -1, LastPositionY = -1;

	WindowWindows() : Handle(nullptr), Parent(nullptr) {}

	virtual IWindow* GetParentWindow() override { return this; }
	virtual bool IsPlatformWindow() const override { return true; };
	virtual Vector2 PositionToAbsolute(Vector2 RelativePosition) const override;
	virtual void SetPosition(Vector2 NewPosition) override;
	virtual void Move(Vector2 Offset) override;
	virtual void SetSize(Vector2 NewSize) override;

	virtual Vector2 GetPosition() const override;
	virtual Vector2 GetSize() const override;

	virtual void UpdatePositionAndSize() override {};

	friend class PlatformWindows;
};
