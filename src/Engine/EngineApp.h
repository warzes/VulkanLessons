#pragma once

#include "RenderSystem.h"

struct EngineCreateInfo final
{
	struct
	{
		int width = 1024;
		int height = 768;
		const wchar_t* title = L"Game";
		bool fullscreen = false;
	} window;

	RenderSystemCreateInfo render;
};

struct EngineAppImpl;

class EngineApp
{
public:
	EngineApp();
	virtual ~EngineApp();

	void Run();

	virtual EngineCreateInfo GetCreateInfo() const { return {}; }
	virtual bool OnCreate() = 0;
	virtual void OnDestroy() = 0;
	virtual void OnUpdate(float deltaTime) = 0;
	virtual void OnFrame() = 0;
	virtual void OnWindowResize();

	void Exit();
	float GetDeltaTime() const;

	RenderSystem* GetRenderSystem() { return m_render; }

private:
	bool create();
	void destroy();
	void frame();
	bool IsEnd() const;
	void setupDPIAwareness();
	void nextFrame();
	std::string getWindowTitle();
	void updateOverlay();
	void render();

	EngineAppImpl* m_data = nullptr;
	RenderSystem* m_render = nullptr;

	bool viewUpdated = false;

	// Frame counter to display fps
	uint32_t frameCounter = 0;
	uint32_t lastFPS = 0;
	std::chrono::time_point<std::chrono::high_resolution_clock> lastTimestamp, tPrevEnd;
	/** @brief Last frame time measured using a high performance timer (if available) */
	float frameTimer = 1.0f;
	bool paused = false;
	// Defines a frame rate independent timer value clamped from -1.0...1.0
	// For use in animations, rotations, etc.
	float timer = 0.0f;
	// Multiplier for speeding up (or slowing down) the global timer
	float timerSpeed = 0.25f;
};