
#include <Windows.h>
#include <windowsx.h>
#include "Widget.h"
#include "PlatformWindows.h"
#include "Render.h"

#include <vector>
#include <algorithm>

namespace
{
	constexpr char* WindowClassName = "DefaultWindowClass";
	std::vector<WindowWindows*> Windows;

	WindowWindows* DraggedWindow = nullptr;
	Vector2 DraggedWindowOffset;
}

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
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
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
	DWORD dwStyle = WS_VISIBLE | /*WS_POPUP | WS_BORDER */ WS_OVERLAPPEDWINDOW;

	HWND NewHWND = CreateWindowA(WindowClassName, Params.Title, dwStyle,
		Params.X, Params.Y, Params.Width, Params.Height,
		Params.Parent ? (HWND)((WindowWindows*)Params.Parent)->Handle : nullptr,
		nullptr, GetModuleHandle(nullptr), nullptr);

	if (NewHWND == nullptr)
		return nullptr;

	WindowWindows* NewWin = new WindowWindows();
	NewWin->Handle = NewHWND;
	NewWin->Parent = Params.Parent;
	NewWin->Canvas = std::move(std::make_unique<Widget>());
	NewWin->Canvas->ParentCanvas = NewWin;
	NewWin->Canvas->SetAnchors(EAnchor::All);
	NewWin->Canvas->SetPivot(Vector2(0.f));
	NewWin->Canvas->SetSize(NewWin->GetSize());
	NewWin->Canvas->Init();

	Windows.push_back(NewWin);

	POINT Cursor;
	GetCursorPos(&Cursor);
	if (Params.bStartDragging)
	{
		DraggedWindow = NewWin;
		DraggedWindowOffset = Vector2((int)Cursor.x - Params.X, (int)Cursor.y - Params.Y);
	}

	ScreenToClient(NewHWND, &Cursor);
	NewWin->LastMouseX = Cursor.x;
	NewWin->LastMouseY = Cursor.y;


	GetRender().OnWindowCreated(NewWin);

	return NewWin;
}

bool PlatformWindows::DestroyWindow(IWindow* pWindow)
{
	if (pWindow)
	{
		auto WindowsCopy = Windows;
		for (auto It : WindowsCopy)
		{
			if (It->Parent == pWindow)
			{
				if (!DestroyWindow(It))
				{
					return false;
				}
			}
		}

		auto It = std::find(Windows.begin(), Windows.end(), pWindow);
		if (It != Windows.end())
		{
			GetRender().OnWindowDestroyed(pWindow);

			Windows.erase(It);
			if (!::DestroyWindow((HWND)((WindowWindows*)pWindow)->Handle))
			{
				return false;
			}
			delete pWindow;
		}

		return true;
	}
	return false;
}

void PlatformWindows::Tick()
{
	MSG msg;

	auto WindowsCopy = Windows;
	for (auto WinIt : WindowsCopy)
	{
		while (PeekMessage(&msg, (HWND)WinIt->Handle, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	if (DraggedWindow)
	{
		POINT Cursor;
		GetCursorPos(&Cursor);
		DraggedWindow->SetPosition(Vector2(Cursor.x, Cursor.y) - DraggedWindowOffset);
	}

	WindowsCopy = Windows;
	for (auto WinIt : WindowsCopy)
	{
		GetRender().PaintWindow(WinIt);
	}
}

LRESULT PlatformWindows::WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (DraggedWindow)
	{
		if(Msg != WM_LBUTTONUP)
			return DefWindowProcA(hWnd, Msg, wParam, lParam);
		DraggedWindow = nullptr;
	}

	switch (Msg)
	{
	case WM_CLOSE:
	{
		auto It = std::find_if(Windows.begin(), Windows.end(), [hWnd](WindowWindows* A) { return A->Handle == (void*)hWnd; });
		if (It != Windows.end())
			GetPlatform().DestroyWindow(*It);
		break;
	}
	case WM_SIZE:
	{
		auto It = std::find_if(Windows.begin(), Windows.end(), [hWnd](WindowWindows* A) { return A->Handle == (void*)hWnd; });
		if (It != Windows.end())
		{
			(*It)->Canvas->SetSize((*It)->GetSize());
		}
		break;
	}
	case WM_LBUTTONDOWN:
	{
		auto It = std::find_if(Windows.begin(), Windows.end(), [hWnd](WindowWindows* A) { return A->Handle == (void*)hWnd; });
		if (It != Windows.end())
		{
			(*It)->LastMouseX = GET_X_LPARAM(lParam);
			(*It)->LastMouseY = GET_Y_LPARAM(lParam);
			if ((*It)->CapturedWidget)
			{
				(*It)->CapturedWidget->OnMousePressed(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 0);
			}
			else
			{
				(*It)->Canvas->OnMousePressed(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 0);
			}
		}
		break;
	}
	case WM_LBUTTONUP:
	{
		auto It = std::find_if(Windows.begin(), Windows.end(), [hWnd](WindowWindows* A) { return A->Handle == (void*)hWnd; });
		if (It != Windows.end())
		{
			if ((*It)->CapturedWidget)
			{
				(*It)->CapturedWidget->OnMouseReleased(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 0);
			}
			else
			{
				(*It)->Canvas->OnMouseReleased(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 0);
			}
		}
		break;
	}
	//case WM_NCHITTEST: 
	//{
	//	LRESULT hit = DefWindowProc(hWnd, Msg, wParam, lParam);
	//	if (hit == HTCLIENT)
	//	{
	//		auto It = std::find_if(Windows.begin(), Windows.end(), [hWnd](WindowWindows* A) { return A->Handle == (void*)hWnd; });
	//		if (It != Windows.end())
	//		{
	//			if ((*It)->bDragging)
	//			{
	//				hit = HTCAPTION;
	//			}
	//		}
	//	}
	//	return hit;
	//}
	case WM_MOUSEMOVE:
	{
		auto It = std::find_if(Windows.begin(), Windows.end(), [hWnd](WindowWindows* A) { return A->Handle == (void*)hWnd; });
		if (It != Windows.end())
		{
			if (!(*It)->bIgnoreMouseMoveEvent)
			{
				if ((*It)->CapturedWidget)
				{
					(*It)->CapturedWidget->OnMouseMoved((*It)->LastMouseX, (*It)->LastMouseY, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
				}
				else
				{
					if ((*It)->LastMouseX != -1 && (*It)->LastMouseX != GET_X_LPARAM(lParam))
						(*It)->Canvas->OnMouseMoved((*It)->LastMouseX, (*It)->LastMouseY, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
				}

				(*It)->LastMouseX = GET_X_LPARAM(lParam);
				(*It)->LastMouseY = GET_Y_LPARAM(lParam);
			}
		}
		break;
	}
	}

	return DefWindowProcA(hWnd, Msg, wParam, lParam);
}

Vector2 WindowWindows::PositionToAbsolute(Vector2 RelativePosition) const
{
	RECT rect;
	GetWindowRect((HWND)Handle, &rect);

	return RelativePosition + Vector2(rect.left, rect.top);
}

Vector2 WindowWindows::GetPosition() const
{
	return Vector2(0.f);
}

Vector2 WindowWindows::GetSize() const
{
	RECT rect;
	GetClientRect((HWND)Handle, &rect);

	return Vector2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));
}

void WindowWindows::SetPosition(Vector2 NewPosition)
{
	SetWindowPos((HWND)Handle, nullptr, (int)NewPosition.x(), (int)NewPosition.y(), 0, 0, SWP_NOSIZE);
}

void WindowWindows::Move(Vector2 Offset)
{
	MSG msg;
	RECT rect;
	GetWindowRect((HWND)Handle, &rect);

	rect.left += (int)Offset.x();
	rect.right += (int)Offset.x();
	rect.top += (int)Offset.y();
	rect.bottom += (int)Offset.y();

	bIgnoreMouseMoveEvent = true;
	MoveWindow((HWND)Handle, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, true);
	UpdateWindow((HWND)Handle);

	LastMouseX += (int)Offset.x();
	LastMouseY += (int)Offset.y();

	bIgnoreMouseMoveEvent = false;
}

void WindowWindows::SetSize(Vector2 NewSize)
{
}

IPlatform& GetPlatform()
{
	static PlatformWindows Platform;
	return Platform;
}
