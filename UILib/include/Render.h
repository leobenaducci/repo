class IWindow;

class IRender
{
public:
	virtual void OnWindowCreated(IWindow*) = 0;
	virtual void OnWindowDestroyed(IWindow*) = 0;	
	virtual void PaintWindow(IWindow*) = 0;
};

IRender& GetRender();