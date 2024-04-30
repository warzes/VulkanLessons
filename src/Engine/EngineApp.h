#pragma once

#include "VulkanApp.h"

struct EngineCreateInfo final
{
	WindowSystemCreateInfo window;
	RenderSystemCreateInfo render;
};

class EngineApp : public VulkanApp
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
	virtual void OnUpdateUIOverlay(vks::UIOverlay* /*overlay*/) {}

	void Exit();
	float GetDeltaTime() const;
	bool& Paused() { return paused; }
	void resize(uint32_t destWidth, uint32_t destHeight);

protected:
	// Frame counter to display fps
	uint32_t frameCounter = 0;
	uint32_t lastFPS = 0;
	std::chrono::time_point<std::chrono::high_resolution_clock> lastTimestamp, tPrevEnd;
	bool paused = false;
	/** @brief Last frame time measured using a high performance timer (if available) */
	float frameTimer = 1.0f;
	// Defines a frame rate independent timer value clamped from -1.0...1.0
	// For use in animations, rotations, etc.
	float timer = 0.0f;
	// Multiplier for speeding up (or slowing down) the global timer
	float timerSpeed = 0.25f;

private:
	bool create();
	bool isEnd() const;
	void frame();
	void nextFrame();
	void render();
	void updateOverlay();
	void destroy();
	std::string getWindowTitle();

	float deltaTime = 0.01f;
};