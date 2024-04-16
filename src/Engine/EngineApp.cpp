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
	default:
		break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}
#endif // _WIN32
//-----------------------------------------------------------------------------
EngineApp::EngineApp()
	: m_data(new EngineAppImpl)
{
}
//-----------------------------------------------------------------------------
EngineApp::~EngineApp()
{
	destroy();
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

	WNDCLASSEX wcex{};
	wcex.cbSize        = sizeof(WNDCLASSEX);
	wcex.style         = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc   = WndProc;
	wcex.hInstance     = m_data->hInstance = GetModuleHandle(nullptr);
	wcex.hIcon         = LoadIcon(nullptr, IDI_APPLICATION);
	wcex.hIconSm       = LoadIcon(nullptr, IDI_WINLOGO);
	wcex.hCursor       = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszClassName = ClassName;
	if (!RegisterClassEx(&wcex))
	{
		Fatal("RegisterClassEx failed.");
		return false;
	}

	m_data->hwnd = CreateWindow(ClassName, createInfo.window.title, WS_OVERLAPPEDWINDOW, 
		CW_USEDEFAULT, CW_USEDEFAULT, createInfo.window.width, createInfo.window.height, nullptr, nullptr, m_data->hInstance, nullptr);
	if (!m_data->hwnd)
	{
		Fatal("CreateWindow failed.");
		return false;
	}

	ShowWindow(m_data->hwnd, SW_SHOW);
	SetForegroundWindow(m_data->hwnd);
	UpdateWindow(m_data->hwnd);
	SetFocus(m_data->hwnd);

	IsExit = false;
	return true;
}
//-----------------------------------------------------------------------------
void EngineApp::destroy()
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