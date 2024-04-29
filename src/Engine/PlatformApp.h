#pragma once

#include "BaseApp.h"
#include "VulkanUIOverlay.h"

struct WindowSystemCreateInfo final
{
	int width = 1024;
	int height = 768;
	const wchar_t* title = L"Game";
	bool fullscreen = false;
};

struct MouseState final
{
	struct {
		bool left = false;
		bool right = false;
		bool middle = false;
	} buttons;
	glm::vec2 position;
};


struct PlatformAppData;

class PlatformApp : public BaseApp
{
public:
	PlatformApp();
	virtual ~PlatformApp() override;

	uint32_t GetFrameWidth() const;
	uint32_t GetFrameHeight() const;
	float GetFrameAspect() const;

	virtual void OnKeyPressed(uint32_t) {}
	virtual void OnKeyUp(uint32_t) {}
	virtual void OnMouseMoved(int32_t x, int32_t y, int32_t dx, int32_t dy) {}
	virtual void OnWindowResize(uint32_t destWidth, uint32_t destHeight) {}

	void SetTitle(const std::string& title);

	void* GetHWND(); // TODO: временно
	void* GetHINSTANCE(); // TODO: временно

	MouseState mouseState; // TODO: временно паблик
	void handleMouseMove(int32_t x, int32_t y);// TODO: временно паблик
	bool resizing = false; // TODO: временно
	void windowResize(uint32_t destWidth, uint32_t destHeight); // TODO: временно паблик
protected:
	bool initPlatformApp(const WindowSystemCreateInfo& createInfo);
	bool windowFrame();
	bool isIconic() const;
	void closePlatformApp();

	vks::UIOverlay UIOverlay; // TODO: потом передвинуть

	bool m_fullscreen = false;
	bool overlay = true;

private:
	void setupDPIAwareness();
	bool initWindow(const WindowSystemCreateInfo& createInfo);
	PlatformAppData* m_data = nullptr;
};