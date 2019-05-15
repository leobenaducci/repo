
#include <Windows.h>
#include "PlatformWindows.h"
#include "Render.h"

#include <vector>
#include <algorithm>

namespace
{
	constexpr char* WindowClassName = "DefaultWindowClass";
	std::vector<WindowWindows*> Windows;
}

static LRESULT WndProc(HWND, UINT, WPARAM, LPARAM);

bool PlatformWindows::SupportsMultipleWindows() const
{
	return true;
}

bool PlatformWindows::RequestingExit() const
{
	return Windows.size() == 0;
}

PlatformWindows::PlatformWindows()
	: IPlatform()
{
	WNDCLASSA wc;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_CLASSDC;
	wc.lpfnWndProc = (WNDPROC)WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetModuleHandle(nullptr);
	wc.hIcon = nullptr;
	wc.hCursor = LoadCursor((HINSTANCE)NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
	wc.lpszMenuName = "";
	wc.lpszClassName = WindowClassName;

	RegisterClassA(&wc);
}

PlatformWindows::~PlatformWindows()
{
}

IWindow* PlatformWindows::NewWindow(struct WINDOW_CREATION_PARAMS& Params)
{
	DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;

	HWND NewHWND = CreateWindowA(WindowClassName, Params.Title, dwStyle,
								0, 0, Params.Width, Params.Height, 
								Params.Parent ? (HWND)((WindowWindows*)Params.Parent)->Handle : nullptr, 
								nullptr, GetModuleHandle(nullptr), nullptr);

	if (NewHWND == nullptr)
		return nullptr;
	
	WindowWindows* NewWin = new WindowWindows();
	NewWin->Handle = NewHWND;
	Windows.push_back(NewWin);

	GetRender().OnWindowCreated(NewWin);

	return NewWin;
}

bool PlatformWindows::DestroyWindow(IWindow* pWindow)
{
	if (pWindow)
	{
		auto It = std::find(Windows.begin(), Windows.end(), pWindow);
		if (It != Windows.end())
		{
			GetRender().OnWindowDestroyed(pWindow);

			Windows.erase(It);
			return ::DestroyWindow((HWND)((WindowWindows*)pWindow)->Handle);
		}
	}
	return false;
}

void PlatformWindows::Tick()
{
	MSG msg;

	for (auto WinIt : Windows)
	{
		while (PeekMessage(&msg, (HWND)WinIt->Handle, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		GetRender().PaintWindow(WinIt);
	}
}

static LRESULT WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
	case WM_CLOSE:
	{
		auto It = std::find_if(Windows.begin(), Windows.end(), [hWnd](WindowWindows* A) { return A->Handle == (void*)hWnd; });
		if (It != Windows.end())
			GetPlatform().DestroyWindow(*It);
		break;
	}
	}

	return DefWindowProcA(hWnd, Msg, wParam, lParam);
}

IPlatform& GetPlatform()
{
	static PlatformWindows Platform;
	return Platform;
}