
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

	int LastMouseX = 0, LastMouseY = 0;
	int LastPositionX = 0, LastPositionY = 0;

	WindowWindows() : Handle(nullptr), Parent(nullptr) {}

	virtual bool IsPlatformWindow() const override { return true; };
	virtual Vector2 GetPosition() const override;
	virtual Vector2 GetSize() const override;
	virtual void UpdatePositionAndSize() override {};

	virtual void SetPosition(Vector2 NewPosition);

	friend class PlatformWindows;
};
