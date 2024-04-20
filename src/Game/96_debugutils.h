#pragma once

/*
Debug utils (VK_EXT_debug_utils)

Shows how to use debug utils for adding labels and colors to Vulkan objects for graphics debuggers. This information helps to identify resources in tools like RenderDoc.

Note: This sample should be run from a graphics debugger like RenderDoc.
*/

class DebugUtilsApp final : public EngineApp
{
public:
	EngineCreateInfo GetCreateInfo() const final { return {}; }
	bool OnCreate() final;
	void OnDestroy() final;
	void OnUpdate(float deltaTime) final;
	void OnFrame() final;
	void OnUpdateUIOverlay(vks::UIOverlay* overlay) final;

	void OnWindowResize(uint32_t destWidth, uint32_t destHeight) final;
	void OnKeyPressed(uint32_t key) final;
	void OnKeyUp(uint32_t key) final;
	void OnMouseMoved(int32_t x, int32_t y, int32_t dx, int32_t dy) final;

private:
	Camera camera;
};