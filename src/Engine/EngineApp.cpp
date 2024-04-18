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
		if (OnCreate())
		{
			m_render->Prepared() = true;
			while (!IsEnd())
			{
				frame();
			}
			m_render->Final();
			OnDestroy();
		}
	}
}
//-----------------------------------------------------------------------------
void EngineApp::Exit()
{
	IsExit = true;
}
//-----------------------------------------------------------------------------
void EngineApp::OnWindowResize()
{
	// TODO:
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

	// TODO: вычислить реальный размер окна
	uint32_t width = createInfo.window.width;
	uint32_t height = createInfo.window.height;
	// TODO: также рендер может менять размер, проверить что там и как это использовать

	if (!m_render->Create(createInfo.render, m_data->hInstance, m_data->hwnd, &width, &height, createInfo.window.fullscreen))
	{
		Fatal("RenderSystem create failed.");
		return false;
	}

	lastTimestamp = std::chrono::high_resolution_clock::now();
	tPrevEnd = lastTimestamp;
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
	while (PeekMessage(&m_data->msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&m_data->msg);
		DispatchMessage(&m_data->msg);
	}
	if (m_render->Prepared() && !IsIconic(m_data->hwnd)) // TODO: может все таки обрабатывать события даже в свернутом окне?
	{
		nextFrame();
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
void EngineApp::nextFrame()
{
	auto tStart = std::chrono::high_resolution_clock::now();
	render();
	frameCounter++;
	auto tEnd = std::chrono::high_resolution_clock::now();
	auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();

	frameTimer = (float)tDiff / 1000.0f;
	OnUpdate(frameTimer);

	// Convert to clamped timer value
	if (!paused)
	{
		timer += timerSpeed * frameTimer;
		if (timer > 1.0)
		{
			timer -= 1.0f;
		}
	}
	float fpsTimer = (float)(std::chrono::duration<double, std::milli>(tEnd - lastTimestamp).count());
	if (fpsTimer > 1000.0f)
	{
		lastFPS = static_cast<uint32_t>((float)frameCounter * (1000.0f / fpsTimer));
#if defined(_WIN32)
		std::string windowTitle = getWindowTitle();
		SetWindowTextA(m_data->hwnd, windowTitle.c_str());
#endif
		frameCounter = 0;
		lastTimestamp = tEnd;
	}
	tPrevEnd = tEnd;

	// TODO: Cap UI overlay update rates
	updateOverlay();
}
//-----------------------------------------------------------------------------
std::string EngineApp::getWindowTitle()
{
	std::string device(GetRenderSystem()->GetDeviceName());
	std::string windowTitle;
	windowTitle = "Game - " + device;
	windowTitle += " - " + std::to_string(frameCounter) + " fps";
	return windowTitle;
}
//-----------------------------------------------------------------------------
void EngineApp::updateOverlay()
{
	/*if (!settings.overlay)
		return;

	ImGuiIO& io = ImGui::GetIO();

	io.DisplaySize = ImVec2((float)width, (float)height);
	io.DeltaTime = frameTimer;

	io.MousePos = ImVec2(mouseState.position.x, mouseState.position.y);
	io.MouseDown[0] = mouseState.buttons.left && UIOverlay.visible;
	io.MouseDown[1] = mouseState.buttons.right && UIOverlay.visible;
	io.MouseDown[2] = mouseState.buttons.middle && UIOverlay.visible;

	ImGui::NewFrame();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
	ImGui::SetNextWindowPos(ImVec2(10 * UIOverlay.scale, 10 * UIOverlay.scale));
	ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
	ImGui::Begin("Vulkan Example", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
	ImGui::TextUnformatted(title.c_str());
	ImGui::TextUnformatted(deviceProperties.deviceName);
	ImGui::Text("%.2f ms/frame (%.1d fps)", (1000.0f / lastFPS), lastFPS);

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 5.0f * UIOverlay.scale));
#endif
	ImGui::PushItemWidth(110.0f * UIOverlay.scale);
	OnUpdateUIOverlay(&UIOverlay);
	ImGui::PopItemWidth();
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
	ImGui::PopStyleVar();
#endif

	ImGui::End();
	ImGui::PopStyleVar();
	ImGui::Render();

	if (UIOverlay.update() || UIOverlay.updated) {
		buildCommandBuffers();
		UIOverlay.updated = false;
	}*/
}
//-----------------------------------------------------------------------------
void EngineApp::render()
{
	OnFrame();
}
//-----------------------------------------------------------------------------