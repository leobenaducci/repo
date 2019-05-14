
#include "Render.h"

class RenderOGL : public IRender
{
public:
	virtual void OnWindowCreated(IWindow*) override;
	virtual void OnWindowDestroyed(IWindow*) override;
	virtual void PaintWindow(IWindow*) override;
};
