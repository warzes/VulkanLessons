#include "stdafx.h"
#include "PlatformApp.h"
#include "Log.h"
//-----------------------------------------------------------------------------
#if defined(_WIN32)
constexpr const wchar_t* ClassName = L"EngineApp";
#endif // _WIN32
//-----------------------------------------------------------------------------
void EngineExit();
bool GetRenderPrepared();
void EngineResize(uint32_t destWidth, uint32_t destHeight);
PlatformApp* pPlatformApp = nullptr;
//-----------------------------------------------------------------------------
#if defined(_WIN32)
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CLOSE:
		EngineExit();
		PostQuitMessage(0);
		break;
		//case WM_DESTROY:
		//	PostQuitMessage(0);
		//	return 0;
	case WM_PAINT:
		ValidateRect(hwnd, nullptr);
		break;
	case WM_KEYDOWN:
		pPlatformApp->OnKeyPressed((uint32_t)wParam);
		break;
	case WM_KEYUP:
		pPlatformApp->OnKeyUp((uint32_t)wParam);
		break;
	case WM_LBUTTONDOWN:
		pPlatformApp->mouseState.position = glm::vec2((float)LOWORD(lParam), (float)HIWORD(lParam));
		pPlatformApp->mouseState.buttons.left = true;
		break;
	case WM_RBUTTONDOWN:
		pPlatformApp->mouseState.position = glm::vec2((float)LOWORD(lParam), (float)HIWORD(lParam));
		pPlatformApp->mouseState.buttons.right = true;
		break;
	case WM_MBUTTONDOWN:
		pPlatformApp->mouseState.position = glm::vec2((float)LOWORD(lParam), (float)HIWORD(lParam));
		pPlatformApp->mouseState.buttons.middle = true;
		break;
	case WM_LBUTTONUP:
		pPlatformApp->mouseState.buttons.left = false;
		break;
	case WM_RBUTTONUP:
		pPlatformApp->mouseState.buttons.right = false;
		break;
	case WM_MBUTTONUP:
		pPlatformApp->mouseState.buttons.middle = false;
		break;
		//case WM_MOUSEWHEEL:
		//{
		//	short wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		//	break;
		//}
	case WM_MOUSEMOVE:
		pPlatformApp->handleMouseMove(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_SIZE:
		if (GetRenderPrepared() && (wParam != SIZE_MINIMIZED))
		{
			if ((pPlatformApp->resizing) || ((wParam == SIZE_MAXIMIZED) || (wParam == SIZE_RESTORED)))
			{
				pPlatformApp->windowResize(LOWORD(lParam), HIWORD(lParam));
			}
		}
		break;
	case WM_GETMINMAXINFO:
	{
		LPMINMAXINFO minMaxInfo = (LPMINMAXINFO)lParam;
		minMaxInfo->ptMinTrackSize.x = 64;
		minMaxInfo->ptMinTrackSize.y = 64;
		break;
	}
	case WM_ENTERSIZEMOVE:
		pPlatformApp->resizing = true;
		break;
	case WM_EXITSIZEMOVE:
		pPlatformApp->resizing = false;
		break;
	default:
		break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}
#endif // _WIN32
//-----------------------------------------------------------------------------
struct PlatformAppData
{
#if defined(_WIN32)
	HINSTANCE hInstance = nullptr;
	HWND hwnd = nullptr;
	MSG msg{};
	uint32_t frameWidth = 0;
	uint32_t frameHeight = 0;
#endif // _WIN32
};
//-----------------------------------------------------------------------------
PlatformApp::PlatformApp()
	: m_data(new PlatformAppData)
{
	pPlatformApp = this;
}
//-----------------------------------------------------------------------------
PlatformApp::~PlatformApp()
{
	delete m_data;
	pPlatformApp = nullptr;
}
//-----------------------------------------------------------------------------
uint32_t PlatformApp::GetFrameWidth() const
{
	return m_data->frameWidth;
}
//-----------------------------------------------------------------------------
uint32_t PlatformApp::GetFrameHeight() const
{
	return m_data->frameHeight;
}
//-----------------------------------------------------------------------------
float PlatformApp::GetFrameAspect() const
{
	return (float)GetFrameWidth() / (float)GetFrameHeight();
}
//-----------------------------------------------------------------------------
bool PlatformApp::initPlatformApp(const WindowSystemCreateInfo& createInfo)
{
	m_data->hInstance = GetModuleHandle(nullptr);

	setupDPIAwareness();

	if (!initWindow(createInfo))
		return false;

	return true;
}
//-----------------------------------------------------------------------------
bool PlatformApp::windowFrame()
{
	while (PeekMessage(&m_data->msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&m_data->msg);
		DispatchMessage(&m_data->msg);
		if (m_data->msg.message == WM_QUIT)
			return false;
	}

	return true;
}
//-----------------------------------------------------------------------------
bool PlatformApp::isIconic() const
{
	return IsIconic(m_data->hwnd);
}
//-----------------------------------------------------------------------------
void PlatformApp::closePlatformApp()
{
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
void PlatformApp::setupDPIAwareness()
{
	typedef HRESULT* (__stdcall* SetProcessDpiAwarenessFunc)(PROCESS_DPI_AWARENESS);
	HMODULE shCore = LoadLibrary(L"Shcore.dll");
	if (shCore)
	{
		SetProcessDpiAwarenessFunc setProcessDpiAwareness = (SetProcessDpiAwarenessFunc)GetProcAddress(shCore, "SetProcessDpiAwareness");

		if (setProcessDpiAwareness != nullptr)
			setProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

		FreeLibrary(shCore);
	}
}
//-----------------------------------------------------------------------------
bool PlatformApp::initWindow(const WindowSystemCreateInfo& createInfo)
{
	m_fullscreen = createInfo.fullscreen;

	WNDCLASSEX wndClass{};
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WndProc;
	wndClass.hInstance = m_data->hInstance;
	wndClass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wndClass.hIconSm = LoadIcon(nullptr, IDI_WINLOGO);
	wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wndClass.lpszClassName = ClassName;
	if (!RegisterClassEx(&wndClass))
	{
		Fatal("RegisterClassEx failed.");
		return false;
	}

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	if (m_fullscreen)
	{
		if ((createInfo.width != (uint32_t)screenWidth) && (createInfo.height != (uint32_t)screenHeight))
		{
			DEVMODE dmScreenSettings;
			memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
			dmScreenSettings.dmSize = sizeof(dmScreenSettings);
			dmScreenSettings.dmPelsWidth = createInfo.width;
			dmScreenSettings.dmPelsHeight = createInfo.height;
			dmScreenSettings.dmBitsPerPel = 32;
			dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
			if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
			{
				if (MessageBox(NULL, L"Fullscreen Mode not supported!\n Switch to window mode?", L"Error", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
				{
					m_fullscreen = false;
				}
				else
				{
					return false;
				}
			}
			screenWidth = createInfo.width;
			screenHeight = createInfo.height;
		}
	}

	DWORD dwExStyle;
	DWORD dwStyle;

	if (m_fullscreen)
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
	windowRect.right = createInfo.fullscreen ? (long)screenWidth : (long)createInfo.width;
	windowRect.bottom = createInfo.fullscreen ? (long)screenHeight : (long)createInfo.height;

	AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

	m_data->hwnd = CreateWindowEx(0, ClassName, createInfo.title, dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		0, 0, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
		nullptr, nullptr, m_data->hInstance, nullptr);

	if (!m_data->hwnd)
	{
		Fatal("Could not create window!");
		return false;
	}

	if (!m_fullscreen)
	{
		// Center on screen
		uint32_t x = (GetSystemMetrics(SM_CXSCREEN) - windowRect.right) / 2;
		uint32_t y = (GetSystemMetrics(SM_CYSCREEN) - windowRect.bottom) / 2;
		SetWindowPos(m_data->hwnd, 0, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	}

	ShowWindow(m_data->hwnd, SW_SHOW);
	SetForegroundWindow(m_data->hwnd);
	SetFocus(m_data->hwnd);

	// TODO: вычислить реальный размер окна
	m_data->frameWidth = createInfo.width;
	m_data->frameHeight = createInfo.height;
	// TODO: также рендер может менять размер, проверить что там и как это использовать

	return true;
}
//-----------------------------------------------------------------------------
void PlatformApp::SetTitle(const std::string& title)
{
	SetWindowTextA(m_data->hwnd, title.c_str());
}
//-----------------------------------------------------------------------------
void* PlatformApp::GetHWND()
{
	return m_data->hwnd;
}
//-----------------------------------------------------------------------------
void* PlatformApp::GetHINSTANCE()
{
	return m_data->hInstance;
}
//-----------------------------------------------------------------------------
void PlatformApp::handleMouseMove(int32_t x, int32_t y)
{
	int32_t dx = (int32_t)mouseState.position.x - x;
	int32_t dy = (int32_t)mouseState.position.y - y;

	bool handled = false;

	if (overlay)
	{
		ImGuiIO& io = ImGui::GetIO();
		handled = io.WantCaptureMouse && UIOverlay.visible;
	}
	if (!handled)
		OnMouseMoved(x, y, dx, dy);

	mouseState.position = glm::vec2((float)x, (float)y);
}
//-----------------------------------------------------------------------------
void PlatformApp::windowResize(uint32_t destWidth, uint32_t destHeight)
{
	if (!GetRenderPrepared())
		return;

	m_data->frameWidth = destWidth;
	m_data->frameHeight = destHeight;

	EngineResize(destWidth, destHeight);

	OnWindowResize(destWidth, destHeight);
}
//-----------------------------------------------------------------------------