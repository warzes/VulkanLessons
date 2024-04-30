#include "stdafx.h"
#include "EngineApp.h"
#include "Log.h"
#include "KeyCodes.h"
//-----------------------------------------------------------------------------
bool IsExit = false;
//-----------------------------------------------------------------------------
EngineApp* currentEngineApp; // TODO: временно
//-----------------------------------------------------------------------------
void EngineExit()
{
	IsExit = true;
	currentEngineApp->RenderPrepared() = false;
}
//-----------------------------------------------------------------------------
bool GetRenderPrepared()
{
	return currentEngineApp->RenderPrepared();
}
//-----------------------------------------------------------------------------
void EngineResize(uint32_t destWidth, uint32_t destHeight)
{
	currentEngineApp->resize(destWidth, destHeight);
}
//-----------------------------------------------------------------------------
EngineApp::EngineApp()
{
	currentEngineApp = this;
}
//-----------------------------------------------------------------------------
EngineApp::~EngineApp()
{
	currentEngineApp = nullptr;
	destroy();
}
//-----------------------------------------------------------------------------
void EngineApp::Run()
{
	if (create())
	{
		if (OnCreate())
		{
			RenderPrepared() = true;
			while (!isEnd())
			{
				frame();
			}
			RenderPrepared() = false;
			renderFinal();
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
float EngineApp::GetDeltaTime() const
{
	return deltaTime;
}
//-----------------------------------------------------------------------------
bool EngineApp::create()
{
	EngineCreateInfo createInfo = GetCreateInfo();

	if (!initPlatformApp(createInfo.window))
		return false;

	if (!initVulkanApp(createInfo.render, createInfo.window.fullscreen))
		return false;

	lastTimestamp = std::chrono::high_resolution_clock::now(); // TODO: перевести в Base?
	tPrevEnd = lastTimestamp;
	IsExit = false;
	return true;
}
//-----------------------------------------------------------------------------
bool EngineApp::isEnd() const
{
	return IsExit;
}
//-----------------------------------------------------------------------------
void EngineApp::frame()
{
	if (!windowFrame())
	{
		IsExit = true;
		return;
	}

	if (RenderPrepared() && !isIconic()) // TODO: может все таки обрабатывать события даже в свернутом окне?
	{
		nextFrame();
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

	deltaTime = frameTimer = (float)tDiff / 1000.0f; // TODO: удалить frameTimer
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
		std::string windowTitle = getWindowTitle();
		SetTitle(windowTitle);

		frameCounter = 0;
		lastTimestamp = tEnd;
	}
	tPrevEnd = tEnd;

	// TODO: Cap UI overlay update rates
	updateOverlay();
}
//-----------------------------------------------------------------------------
void EngineApp::render()
{
	if (RenderPrepared())
		OnFrame();
}
//-----------------------------------------------------------------------------
void EngineApp::updateOverlay()
{
	if (!overlay)
		return;

	ImGuiIO& io = ImGui::GetIO();

	io.DisplaySize = ImVec2((float)destWidth, (float)destHeight);
	io.DeltaTime = frameTimer;

	io.MousePos = ImVec2(mouseState.position.x, mouseState.position.y);
	io.MouseDown[0] = mouseState.buttons.left && UIOverlay.visible;
	io.MouseDown[1] = mouseState.buttons.right && UIOverlay.visible;
	io.MouseDown[2] = mouseState.buttons.middle && UIOverlay.visible;

	ImGui::NewFrame();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
	ImGui::SetNextWindowPos(ImVec2(10 * UIOverlay.scale, 10 * UIOverlay.scale));
	ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
	ImGui::Begin("Game", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
	ImGui::TextUnformatted("Game");
	ImGui::TextUnformatted(m_physicalDevice.deviceProperties.deviceName);
	ImGui::Text("%.2f ms/frame (%.1d fps)", (1000.0f / lastFPS), lastFPS);
	ImGui::PushItemWidth(110.0f * UIOverlay.scale);
	OnUpdateUIOverlay(&UIOverlay);
	ImGui::PopItemWidth();

	ImGui::End();
	ImGui::PopStyleVar();
	ImGui::Render();

	if (UIOverlay.update() || UIOverlay.updated) 
	{
		buildCommandBuffers();
		UIOverlay.updated = false;
	}
}
//-----------------------------------------------------------------------------
void EngineApp::destroy()
{
	closeVulkanApp();
	closePlatformApp();
}
//-----------------------------------------------------------------------------
std::string EngineApp::getWindowTitle()
{
	std::string device(GetDeviceName());
	std::string windowTitle;
	windowTitle = "Game - " + device;
	windowTitle += " - " + std::to_string(frameCounter) + " fps";
	return windowTitle;
}
//-----------------------------------------------------------------------------
void EngineApp::resize(uint32_t destWidth, uint32_t destHeight)
{
	resizeRender(destWidth, destHeight);	
}
//-----------------------------------------------------------------------------