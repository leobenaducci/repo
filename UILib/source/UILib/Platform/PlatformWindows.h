
#include <Windows.h>
#include "Platform.h"

class PlatformWindows : public IPlatform
{
public:
	virtual bool SupportsMultipleWindows() const override;
	virtual bool RequestingExit() const override;

	PlatformWindows();
	virtual ~PlatformWindows();

	virtual IWindow* NewWindow(struct UI_WINDOW_CREATION_PARAMS&) override;
	virtual bool DestroyWindow(IWindow*) override;
	
	virtual void Tick() override;
};

class WindowWindows : public IWindow
{
public:
	void* Handle;

	WindowWindows() : Handle(nullptr) {}
};
