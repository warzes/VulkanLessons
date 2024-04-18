#include "stdafx.h"
#include "EngineApp.h"
#include "Log.h"
//-----------------------------------------------------------------------------
#if defined(_WIN32)
constexpr const wchar_t* ClassName = L"EngineApp";
#endif // _WIN32
bool IsExit = false;
//-----------------------------------------------------------------------------
struct EngineAppImpl
{
#if defined(_WIN32)
	HINSTANCE hInstance = nullptr;
	HWND hwnd = nullptr;
	MSG msg{};
#endif // _WIN32

	float deltaTime = 0.01f;
};
//-----------------------------------------------------------------------------
#if defined(_WIN32)
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CLOSE:
		IsExit = true;
		PostQuitMessage(0);
		break;
	//case WM_DESTROY:
	//	PostQuitMessage(0);
	//	return 0;
	case WM_PAINT:
		ValidateRect(hwnd, nullptr);
		break;
	case WM_GETMINMAXINFO:
		{
			LPMINMAXINFO minMaxInfo = (LPMINMAXINFO)lParam;
			minMaxInfo->ptMinTrackSize.x = 64;
			minMaxInfo->ptMinTrackSize.y = 64;
			break;
		}
	default:
		break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}
#endif // _WIN32
//-----------------------------------------------------------------------------
EngineApp::EngineApp()
	: m_data(new EngineAppImpl)
	, m_render(new RenderSystem)
{
}
//-----------------------------------------------------------------------------
EngineApp::~EngineApp()
{
	destroy();
	delete m_render;
	delete m_data;
}
//-----------------------------------------------------------------------------
void EngineApp::Run()
{
	if (create())
	{
		while (!IsEnd())
		{
			frame();
		}
	}
}
//-----------------------------------------------------------------------------
void EngineApp::Exit()
{
	IsExit = true;
}
//-----------------------------------------------------------------------------
float EngineApp::GetDeltaTime() const
{
	return m_data->deltaTime;
}
//-----------------------------------------------------------------------------
bool EngineApp::create()
{
	EngineCreateInfo createInfo = GetCreateInfo();
	m_data->hInstance = GetModuleHandle(nullptr);

	setupDPIAwareness();

	WNDCLASSEX wndClass{};
	wndClass.cbSize        = sizeof(WNDCLASSEX);
	wndClass.style         = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc   = WndProc;
	wndClass.hInstance     = m_data->hInstance;
	wndClass.hIcon         = LoadIcon(nullptr, IDI_APPLICATION);
	wndClass.hIconSm       = LoadIcon(nullptr, IDI_WINLOGO);
	wndClass.hCursor       = LoadCursor(nullptr, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wndClass.lpszClassName = ClassName;
	if (!RegisterClassEx(&wndClass))
	{
		Fatal("RegisterClassEx failed.");
		return false;
	}

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	if (createInfo.window.fullscreen)
	{
		if ((createInfo.window.width != (uint32_t)screenWidth) && (createInfo.window.height != (uint32_t)screenHeight))
		{
			DEVMODE dmScreenSettings;
			memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
			dmScreenSettings.dmSize = sizeof(dmScreenSettings);
			dmScreenSettings.dmPelsWidth = createInfo.window.width;
			dmScreenSettings.dmPelsHeight = createInfo.window.height;
			dmScreenSettings.dmBitsPerPel = 32;
			dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
			if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
			{
				if (MessageBox(NULL, L"Fullscreen Mode not supported!\n Switch to window mode?", L"Error", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
				{
					createInfo.window.fullscreen = false;
				}
				else
				{
					return false;
				}
			}
			screenWidth = createInfo.window.width;
			screenHeight = createInfo.window.height;
		}
	}

	DWORD dwExStyle;
	DWORD dwStyle;

	if (createInfo.window.fullscreen)
	{
		dwExStyle = WS_EX_APPWINDOW;
		dwStyle = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	}
	else
	{
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	}

	RECT windowRect;
	windowRect.left = 0L;
	windowRect.top = 0L;
	windowRect.right = createInfo.window.fullscreen ? (long)screenWidth : (long)createInfo.window.width;
	windowRect.bottom = createInfo.window.fullscreen ? (long)screenHeight : (long)createInfo.window.height;

	AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

	m_data->hwnd = CreateWindowEx(0, ClassName, createInfo.window.title, dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		0, 0,  windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, 
		nullptr, nullptr, m_data->hInstance, nullptr);

	if (!m_data->hwnd)
	{
		Fatal("Could not create window!");
		return false;
	}

	if (!createInfo.window.fullscreen)
	{
		// Center on screen
		uint32_t x = (GetSystemMetrics(SM_CXSCREEN) - windowRect.right) / 2;
		uint32_t y = (GetSystemMetrics(SM_CYSCREEN) - windowRect.bottom) / 2;
		SetWindowPos(m_data->hwnd, 0, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	}

	ShowWindow(m_data->hwnd, SW_SHOW);
	SetForegroundWindow(m_data->hwnd);
	SetFocus(m_data->hwnd);

	if (!m_render->Create(createInfo.render))
	{
		Fatal("RenderSystem create failed.");
		return false;
	}

	IsExit = false;
	return true;
}
//-----------------------------------------------------------------------------
void EngineApp::destroy()
{
	m_render->Destroy();
	if (m_data->hwnd)
	{
		DestroyWindow(m_data->hwnd);
		m_data->hwnd = nullptr;
	}
	if (m_data->hInstance)
	{
		UnregisterClass(ClassName, m_data->hInstance);
		m_data->hInstance = nullptr;
	}
}
//-----------------------------------------------------------------------------
void EngineApp::frame()
{
	while (GetMessage(&m_data->msg, nullptr, 0, 0))
	{
		TranslateMessage(&m_data->msg);
		DispatchMessage(&m_data->msg);
	}
}
//-----------------------------------------------------------------------------
bool EngineApp::IsEnd() const
{
	return IsExit;
}
//-----------------------------------------------------------------------------
void EngineApp::setupDPIAwareness()
{
	typedef HRESULT* (__stdcall* SetProcessDpiAwarenessFunc)(PROCESS_DPI_AWARENESS);
	HMODULE shCore = LoadLibraryA("Shcore.dll");
	if (shCore)
	{
		SetProcessDpiAwarenessFunc setProcessDpiAwareness = (SetProcessDpiAwarenessFunc)GetProcAddress(shCore, "SetProcessDpiAwareness");

		if (setProcessDpiAwareness != nullptr)
			setProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

		FreeLibrary(shCore);
	}
}
//-----------------------------------------------------------------------------