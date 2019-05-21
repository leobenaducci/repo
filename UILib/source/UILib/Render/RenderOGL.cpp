
#include <Windows.h>
#include <gl/GL.h>
#include "RenderOGL.h"
#include "Vector.h"

#include "../Platform/PlatformWindows.h"
#include "Widget.h"

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

IRender& GetRender()
{
	static RenderOGL Render;
	return Render;
}

void RenderOGL::OnWindowCreated(IWindow* pWindow)
{
	Vector2 a(0), b(1);
	Vector2 c = a + b;

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
	OGL_WINDOW_DATA& Data = Windows[pWindow];
	wglDeleteContext(Data.hRC);
	Windows.erase(pWindow);
}

void RenderOGL::PaintWindow(IWindow* pWindow)
{
	OGL_WINDOW_DATA& Data = Windows[pWindow];
	wglMakeCurrent(Data.hDC, Data.hRC);

	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, (GLsizei)pWindow->GetSize().x(), (GLsizei)pWindow->GetSize().y());
	glOrtho(0, pWindow->GetSize().x(), pWindow->GetSize().y(), 0, -1000, 1000);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	pWindow->GetCanvas()->Render();

	SwapBuffers(Data.hDC);

	wglMakeCurrent(nullptr, nullptr);
}

void RenderOGL::DrawLine(const Vector2& Start, const Vector2& End, const Vector4& Color)
{
	glBegin(GL_LINES);
	glColor4fv(Color.v);
	glVertex2fv(Start.v);
	glVertex2fv(End.v);
	glEnd();
}

void RenderOGL::DrawRect(const Vector2& Pos, const Vector2& Size, const Image& Image, const Vector4& Color)
{
	glBegin(GL_QUADS);
	glColor4fv(Color.v);
	glVertex2fv((Pos + Vector2(0,0)).v);
	glVertex2fv((Pos + Vector2(Size.x(), 0)).v);
	glVertex2fv((Pos + Vector2(Size.x(), Size.y())).v);
	glVertex2fv((Pos + Vector2(0, Size.y())).v);
	glEnd();
}

void RenderOGL::DrawString(const Vector2& Pos, const Font& Font, wchar_t* Text)
{
}

void RenderOGL::DrawLines(const std::vector<RenderOGL::Vertex>& Vertices, bool bStrip)
{
	glBegin(bStrip ? GL_LINE_STRIP : GL_LINES);
	for (const auto& It : Vertices)
	{
		glColor4fv(It.Color.v);
		glTexCoord2fv(It.UV.v);
		glVertex3fv(It.Pos.v);
	}
	glEnd();
}

void RenderOGL::DrawPoly(const std::vector<RenderOGL::Vertex>& Vertices)
{
	glBegin(GL_TRIANGLES);
	for (const auto& It : Vertices)
	{
		glColor4fv(It.Color.v);
		glTexCoord2fv(It.UV.v);
		glVertex3fv(It.Pos.v);
	}
	glEnd();
}

void RenderOGL::SetClipRect(const Vector2& Min, const Vector2& Max)
{
	glScissor((size_t)Min.x(), (size_t)Min.y(), (size_t)Max.x() - (size_t)Min.x(), (size_t)Max.y() - (size_t)Min.y());
}

