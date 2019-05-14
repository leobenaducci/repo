
#include <Windows.h>
#include <gl/GL.h>
#include "RenderOGL.h"

#include "../Platform/PlatformWindows.h"

#include <map>

#pragma comment(lib, "opengl32.lib")

namespace
{
	struct OGL_WINDOW_DATA
	{
		HDC hDC;
		HGLRC hRC;
	};
	std::map<IWindow*, OGL_WINDOW_DATA> Windows;
}

void RenderOGL::OnWindowCreated(IWindow* pWindow)
{
	WindowWindows* pWin = (WindowWindows*)pWindow;

	OGL_WINDOW_DATA Data;
	Data.hDC = GetDC((HWND)pWin->Handle);

	unsigned int l_PixelFormat = 0;

	// some pixel format descriptor that I generally use:
	static PIXELFORMATDESCRIPTOR l_Pfd = { sizeof(PIXELFORMATDESCRIPTOR), 1,
		PFD_DRAW_TO_WINDOW + PFD_SUPPORT_OPENGL + PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA, 32, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 16, 0, 0, PFD_MAIN_PLANE, 0, 0, 0, 0 };

	if (!(l_PixelFormat = ChoosePixelFormat(Data.hDC, &l_Pfd)))
		return;

	if (!SetPixelFormat(Data.hDC, l_PixelFormat, &l_Pfd))
		return;

	Data.hRC = wglCreateContext(Data.hDC);

	Windows[pWindow] = Data;
}

void RenderOGL::OnWindowDestroyed(IWindow* pWindow)
{
	Windows.erase(pWindow);
}

void RenderOGL::PaintWindow(IWindow* pWindow)
{
	OGL_WINDOW_DATA& Data = Windows[pWindow];
	wglMakeCurrent(Data.hDC, Data.hRC);

	glClear(GL_COLOR_BUFFER_BIT);
	SwapBuffers(Data.hDC);

	wglMakeCurrent(nullptr, nullptr);
}

IRender& GetRender()
{
	static RenderOGL Render;
	return Render;
}
