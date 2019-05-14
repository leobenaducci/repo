
struct UI_WINDOW_CREATION_PARAMS
{
	char* Title = "Window";
	unsigned int X = 0;
	unsigned int Y = 0;
	unsigned int Width = 0;
	unsigned int Height = 0;
	class IWindow* Parent = nullptr;
};

class IWindow
{
};

class IPlatform
{
public:
	virtual bool SupportsMultipleWindows() const = 0;
	virtual bool RequestingExit() const = 0;

	virtual IWindow* NewWindow(struct UI_WINDOW_CREATION_PARAMS&) = 0;
	virtual bool DestroyWindow(IWindow*) = 0;
	
	virtual void Tick() = 0;
};

IPlatform& GetPlatform();
